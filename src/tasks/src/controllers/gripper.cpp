#include "tasks/controllers/task_factory.hpp"
#include "tasks/task_protocol.hpp"

#include <utility>

Gripper::Gripper(
  std::shared_ptr<tf2_ros::Buffer> tf_buffer,
  const GripperConfig & config,
  const rclcpp::Time start_time,
  bool drop)
  : Task(std::move(tf_buffer)), drop_(drop), timeout_(start_time), config_(config) {
  command_.x = 0.1;
  timeout_ = timeout_ + rclcpp::Duration::from_seconds(config_.task_timeout_s);
}

int Gripper::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  if (msg->observations.empty()) {
    return 0;
  }

  const int target_angle = msg->observations[0].bearing;
  rcl_clock_type_t node_clock_type = this->timeout_.get_clock_type();

  rclcpp::Time msg_time(msg->header.stamp, node_clock_type);
  if (msg_time > timeout_) {
    RCLCPP_WARN(rclcpp::get_logger("Gripper"), "Gripper task timed out");
    return task_protocol::kTaskComplete;
  }

  command_ = {0.0, 0.0, 0.0};
  if (target_angle == 1000) {
    command_.x = 1.0;
    return 1;
  }
  const int angle_deadband = drop_ ? config_.drop_angle_deadband_deg : config_.pickup_angle_deadband_deg;
  int target_angle_normalized = ((target_angle + 180) % 360);
  target_angle_normalized = target_angle_normalized > 180 ? target_angle_normalized - 360 : target_angle_normalized;

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