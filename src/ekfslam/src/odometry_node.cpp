#include <memory>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "mavros_msgs/msg/rc_out.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2/utils.h>

#include "ekfslam/odometry.hpp"

class OdometryNode : public rclcpp::Node {
public:
  OdometryNode() : Node("odometry") {
    Odometry::Config cfg;
    this->declare_parameter("mass", cfg.mass);
    this->declare_parameter("width", cfg.width);
    this->declare_parameter("drag_lin", cfg.drag_lin);
    this->declare_parameter("thrust_k_f", cfg.thrust_k_f);
    this->declare_parameter("thrust_k_r", cfg.thrust_k_r);
    this->declare_parameter("rc_lag", cfg.rc_lag);
    this->declare_parameter("xy_dist_noise_scaler", cfg.xy_dist_noise_scaler);
    this->declare_parameter("r_yaw", cfg.r_yaw);

    cfg.mass = this->get_parameter("mass").as_double();
    cfg.width = this->get_parameter("width").as_double();
    cfg.drag_lin = this->get_parameter("drag_lin").as_double();
    cfg.thrust_k_f = this->get_parameter("thrust_k_f").as_double();
    cfg.thrust_k_r = this->get_parameter("thrust_k_r").as_double();
    cfg.rc_lag = this->get_parameter("rc_lag").as_double();
    cfg.xy_dist_noise_scaler = this->get_parameter("xy_dist_noise_scaler").as_double();
    cfg.r_yaw = this->get_parameter("r_yaw").as_double();

    odom_ = std::make_unique<Odometry>(cfg);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    auto qos_latest = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort();

    rc_sub_ = this->create_subscription<mavros_msgs::msg::RCOut>(
      "/mavros/rc/out", qos_latest,
      [this](const mavros_msgs::msg::RCOut::SharedPtr msg) {
        if (msg->channels.size() > 4) {
          this->rc_l_ = msg->channels[1];
          this->rc_r_ = msg->channels[0];
          this->rc_back_ = msg->channels[4];
        }
      });

    imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
      "/mavros/imu/data", qos_latest,
      [this](const sensor_msgs::msg::Imu::SharedPtr msg) {
        this->imu_callback(msg);
      });

    RCLCPP_INFO(this->get_logger(), "Odometry node initialized");
  }

private:
  void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    double curtime = msg->header.stamp.sec + msg->header.stamp.nanosec * 1e-9;
    if (last_imu_time_ == 0.0) {
      last_imu_time_ = curtime;
      return;
    }

    double dt = curtime - last_imu_time_;
    last_imu_time_ = curtime;

    if (dt <= 0.0 || dt > 1.0) {
      RCLCPP_WARN(this->get_logger(), "IMU Time Glitch Detected! dt = %f. Skipping update.", dt);
      return;
    }

    double yaw = tf2::getYaw(msg->orientation);
    double yaw_cov = msg->orientation_covariance[8];

    odom_->update_physics(rc_l_, rc_r_, rc_back_, dt, yaw, yaw_cov);

    Eigen::Vector3d p = odom_->get_pose();

    geometry_msgs::msg::TransformStamped ts;
    ts.header.stamp = msg->header.stamp;
    ts.header.frame_id = "odom";
    ts.child_frame_id = "base_link";
    ts.transform.translation.x = p(0);
    ts.transform.translation.y = p(1);
    ts.transform.translation.z = 0.0;

    tf2::Quaternion q;
    q.setRPY(0, 0, p(2));
    ts.transform.rotation = tf2::toMsg(q);

    tf_broadcaster_->sendTransform(ts);
  }

  std::unique_ptr<Odometry> odom_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  rclcpp::Subscription<mavros_msgs::msg::RCOut>::SharedPtr rc_sub_;
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;

  int rc_l_ = 1500;
  int rc_r_ = 1500;
  int rc_back_ = 1500;
  double last_imu_time_ = 0.0;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<OdometryNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
