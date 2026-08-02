#ifndef EKF_SLAM_HPP
#define EKF_SLAM_HPP

#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <Eigen/Dense>
#include <mutex>
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "std_msgs/msg/header.hpp"

#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2/utils.h>
#include "interfaces/msg/feature_observation.hpp"
#include "interfaces/msg/feature_observations.hpp"
#include "std_srvs/srv/set_bool.hpp"


/**
 * @class EKFSLAM
 * @brief ROS 2 Node for AUV EKF SLAM with a fixed 13-feature map.
 */
class EKFSLAM : public rclcpp::Node {
public:
    EKFSLAM();
    /**
     * @brief Applies a rigid relative constraint between two landmarks in global coordinates.
     * @param id_a ID of the reference feature.
     * @param id_b ID of the target feature.
     * @param dx Global X distance between them (B - A).
     * @param dy Global Y distance between them (B - A).
    * @param y_noise Measurement noise for the constraint along the Y axis.
    */
    void apply_relative_constraint(int id_a, int id_b, double dx, double dy, double y_noise=1e-6);
    
    /**
     * @brief EKF Prediction step using relative odom->base_link TF updates.
     */
    void predict();

    /**
     * @brief EKF Update step using feature observations
     * 
     * @param features Batch of feature observations
     */
    void batch_update(const interfaces::msg::FeatureObservations::SharedPtr features);

    /**
     * @brief Publishes TF transforms for the robot and features.
     */
    void publish_transforms();
private:
    std::mutex state_mutex_;
    static constexpr int feature_count_ = 13;
    static constexpr int state_size_ = 3 + (2 * feature_count_) + 3; // robot pose + landmarks + delayed pose
    std::vector<std::string> features_;
    std::vector<bool> feature_seen_;
    Eigen::Matrix<double, state_size_, 1> state_mu_;
    Eigen::Matrix<double, state_size_, state_size_> state_cov_;
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_{nullptr};
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    Eigen::Vector3d prev_odom_pose_;
    bool has_prev_odom_pose_ = false;
    double xy_dist_noise_scaler_ = 0.2;
    double r_yaw_ = 0.00005;
    rclcpp::Subscription<interfaces::msg::FeatureObservations>::SharedPtr feature_sub_;
    rclcpp::Publisher<interfaces::msg::FeatureObservations>::SharedPtr feature_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    double bearing_std_dev_ = 0.01; // Base bearing standard deviation.
    double bearing_range_scale_ = 0.0005;
    double association_tolerance_rad_ = 0.05;
    double new_association_min_rad_ = 20.0 * M_PI / 180.0;
    double snapshot_time_tolerance_s_ = 0.001;
    double imu_yaw_std_dev_deg_ = 0.5;
    bool use_imu_update_ = true;
    // Flare association and initialization tuning loaded from params.
    double flare_association_factor_ = 0.6;
    double new_flare_dist_ = 5.0;
    double new_flare_cov_depth_ = 0.8;
    double new_flare_cov_perp_ = 0.2;
    bool buckets_locked_ = false;

    // --- Callback Groups ---
    // Handles state timer
    rclcpp::CallbackGroup::SharedPtr state_cbg_;
    
    // Handles heavy CV: Frame Trigger and Feature Observations
    rclcpp::CallbackGroup::SharedPtr vision_cbg_;

    // Delayed pose state used for frame-triggered visual updates.
    rclcpp::Subscription<std_msgs::msg::Header>::SharedPtr trigger_sub_;
    double snapshot_time_ = 0.0;
    bool snapshot_ready_ = false;
    int past_idx_ = state_size_ - 3; // Last three states store delayed robot pose.
    void trigger_callback(const std_msgs::msg::Header::SharedPtr msg);

    // Whether EKFSLAM is actively integrating odometry and running predictions.
    bool active_ = false;
    rclcpp::Service<std_srvs::srv::SetBool>::SharedPtr activate_srv_;

    static double normalize_angle(double a);
    
    /**
     * @brief Generates a 2x2 covariance matrix from an ellipse.
     * @param std_x Major axis standard deviation.
     * @param std_y Minor axis standard deviation.
     * @param theta Rotation in radians.
     * @return Eigen::Matrix2d Rotated covariance matrix.
     */
    static Eigen::Matrix2d ellipse_covariance(double std_x, double std_y, double theta=0.0);
    
    double get_bearing_var(double range) const;

    /**
    * @brief Applies soft/hard positional constraints to active flare landmarks.
     */
    void apply_active_flare_constraints();

    double association_check(interfaces::msg::FeatureObservation &obs, const std::vector<int> & candidate_ids, double override_factor=1.0, bool allow_unseen=false);
    
    /**
     * @brief Initializes a feature in the state vector with an initial guess.
     * @param id Feature ID (0-12).
     * @param x Estimated global X position.
     * @param y Estimated global Y position.
     * @param cov Initial confidence/uncertainty matrix.
     */
    void init_feature(int id, double x, double y, const Eigen::Matrix2d &cov);

    /**
     * @brief Re-initializes an unseen flare landmark along the observed bearing ray.
     *
     * Used only when association selected an unseen flare slot. Keeps the same
     * state/covariance update sequence that previously lived inline in batch_update.
     */
    void reinitialize_unseen_flare(int flare_id, double observed_bearing);
    
    /**
     * @brief Publishes TF transforms for the robot and features.
     */
    void publish_tf(const std::string &frame, const std::string &child, const Eigen::Vector3d &p, const rclcpp::Time &t);
};
#endif