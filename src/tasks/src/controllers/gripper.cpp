#include "tasks/controllers/task_factory.hpp"

#include <utility>

Gripper::Gripper(
  std::shared_ptr<tf2_ros::Buffer> tf_buffer,
  const GripperConfig & config,
  const rclcpp::Time start_time,
  bool drop)
  : Task(std::move(tf_buffer)), drop_(drop), timeout_(start_time), config_(config) {

  // if (timeout_ == rclcpp::Time()) {
  //   timeout_ = rclcpp::Clock(RCL_ROS_TIME).now();
  // }
  command_.x = 0.1;
  timeout_ = timeout_ + rclcpp::Duration::from_seconds(config_.task_timeout_s);
}

int Gripper::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  if (msg->observations.empty()) {
    return 0;
  }

  const int target_angle = msg->observations[0].bearing;
  // 1. Extract the clock type from the start_time you already saved
  rcl_clock_type_t node_clock_type = this->timeout_.get_clock_type();

  // 2. Apply that clock type when converting the message timestamp
  rclcpp::Time msg_time(msg->header.stamp, node_clock_type);
  if (msg_time > timeout_) {
    RCLCPP_WARN(rclcpp::get_logger("Gripper"), "Gripper task timed out");
    return -1;
  }

  command_ = {0.0, 0.0, 0.0};
  if (target_angle == 1000) {
    command_.x = 1.0;
    return 1;
  }
  const int angle_deadband = drop_ ? config_.drop_angle_deadband_deg : config_.pickup_angle_deadband_deg;
  int target_angle_normalized = ((target_angle + 180) % 360); // Normalize to [0, 360)
  target_angle_normalized = target_angle_normalized > 180 ? target_angle_normalized - 360 : target_angle_normalized; // Normalize to [-180, 180)

  if (abs(target_angle_normalized) < angle_deadband) {
    if (target_angle >= 360) {
      command_.x = 0.5;
    } else {
      return -1;
    }
  } else {
    command_.yaw = target_angle_normalized > 0 ? 1.0 : -1.0;
  }

  return 1;
}