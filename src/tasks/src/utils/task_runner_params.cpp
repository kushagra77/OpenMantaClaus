#include "tasks/utils/task_runner_params.hpp"

#include <cmath>

TaskRunnerParams load_task_runner_params(rclcpp::Node & node)
{
  static const std::vector<std::string> kDefaultFeatureNames = {
    "flag", "gate_left", "gate_right", "flare_1", "flare_2", "flare_3",
    "bucket_1", "bucket_2", "bucket_3", "bucket_4", "aruco_marker",
    "qual_gate_left", "qual_gate_right"
  };
  static const std::vector<int64_t> kDefaultFeatureIndices = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
  };

  constexpr double kRadToDeg = 180.0 / M_PI;
  constexpr double kDegToRad = M_PI / 180.0;

  TaskRunnerParams params;
  params.feature_names = kDefaultFeatureNames;
  params.feature_indices = kDefaultFeatureIndices;

  node.declare_parameter<std::vector<std::string>>("common.feature_names", params.feature_names);
  node.declare_parameter<std::vector<int64_t>>("common.feature_indices", params.feature_indices);
  node.declare_parameter<bool>("debug_flag", params.debug_flag);
  node.declare_parameter<std::string>("default_task", params.default_task);
  node.declare_parameter<double>("control_frequency_hz", params.control_frequency_hz);
  node.declare_parameter<double>("default_speed", params.default_speed);
  node.declare_parameter<double>("max_turn_speed", params.max_turn_speed);
  node.declare_parameter<double>("aggressive_turn_angle_deg", params.aggressive_turn_angle_rad * kRadToDeg);
  node.declare_parameter<double>("kp_turn", params.kp_turn);
  node.declare_parameter<double>("kd_turn", params.kd_turn);
  node.declare_parameter<double>("fps_log_period_s", params.fps_log_period_s);
  node.declare_parameter<int>("servo_90_pwm_diff", params.servo_90_pwm_diff);
  node.declare_parameter<double>("gripper_linear_speed", params.gripper_linear_speed);
  node.declare_parameter<double>("gripper_yaw_speed", params.gripper_yaw_speed);
  node.declare_parameter<double>("servo_sleep", params.servo_sleep);
  node.declare_parameter<double>("pickup_sleep", params.pickup_sleep);
  node.declare_parameter<double>("stop_cooldown_s", params.stop_cooldown_s);
  node.declare_parameter<double>("task.target_gain", params.task_config.target_gain);
  node.declare_parameter<double>("task.repellant_gain", params.task_config.repellant_gain);
  node.declare_parameter<double>("task.repellant_range", params.task_config.repellant_range);
  node.declare_parameter<double>("task.repellant_ellipse_x", params.task_config.repellant_ellipse_x);
  node.declare_parameter<double>("task.repellant_ellipse_y", params.task_config.repellant_ellipse_y);
  node.declare_parameter<double>("task.repellant_passed_margin_rad", params.task_config.repellant_passed_margin_rad);
  node.declare_parameter<double>("gripper.task_timeout_s", params.gripper_config.task_timeout_s);
  node.declare_parameter<double>("gripper.drop_angle_deadband_deg", params.gripper_config.drop_angle_deadband_deg);
  node.declare_parameter<double>("gripper.pickup_angle_deadband_deg", params.gripper_config.pickup_angle_deadband_deg);
  node.declare_parameter<double>("flare.scan_target_reached_threshold_m", params.flare_config.scan_target_reached_threshold_m);
  node.declare_parameter<double>("flare.target_reached_threshold_m", params.flare_config.target_reached_threshold_m);
  node.declare_parameter<double>("aruco.target_reached_threshold_m", params.aruco_config.target_reached_threshold_m);
  node.declare_parameter<double>("buckets.target_reached_threshold_m", params.buckets_config.target_reached_threshold_m);
  node.declare_parameter<double>("buckets.lock_bucket_proximity_threshold_m", params.buckets_config.lock_bucket_proximity_threshold_m);
  node.declare_parameter<double>("gate.ellipse_x", params.gate_config.ellipse_x);
  node.declare_parameter<double>("gate.ellipse_y", params.gate_config.ellipse_y);
  node.declare_parameter<double>("gate.forward_exit_margin_m", params.gate_config.forward_exit_margin_m);
  node.declare_parameter<double>("gate.apf_target_x_offset_m", params.gate_config.apf_target_x_offset_m);
  node.declare_parameter<double>("gate.gate_gain", params.gate_config.gate_gain);
  node.declare_parameter<double>("gate.blind_gate_gain_factor", params.gate_config.blind_gate_gain_factor);
  node.declare_parameter<double>("gate.repellant_gain", params.gate_config.repellant_gain);
  node.declare_parameter<double>("gate.repellant_range", params.gate_config.repellant_range);
  node.declare_parameter<double>("gate.repellant_ellipse_x", params.gate_config.repellant_ellipse_x);
  node.declare_parameter<double>("gate.repellant_ellipse_y", params.gate_config.repellant_ellipse_y);
  node.declare_parameter<double>("gate.repellant_passed_margin_m", params.gate_config.repellant_passed_margin_m);
  node.declare_parameter<double>("qual_gate.angle_tolerance_deg", params.qual_gate_config.angle_tolerance_rad * kRadToDeg);
  node.declare_parameter<double>("qual_gate.slam_trust", params.qual_gate_config.slam_trust);
  node.declare_parameter<double>("qual_gate.ellipse_x", params.qual_gate_config.ellipse_x);
  node.declare_parameter<double>("qual_gate.ellipse_y", params.qual_gate_config.ellipse_y);
  node.declare_parameter<double>("qual_gate.forward_exit_margin_m", params.qual_gate_config.forward_exit_margin_m);
  node.declare_parameter<double>("qual_gate.backward_exit_margin_m", params.qual_gate_config.backward_exit_margin_m);
  node.declare_parameter<double>(
    "qual_gate.turn_target_yaw_offset_deg",
    params.qual_gate_config.turn_target_yaw_offset_rad * kRadToDeg);
  node.declare_parameter<double>("qual_gate.apf_target_x_offset_m", params.qual_gate_config.apf_target_x_offset_m);

  params.feature_names = node.get_parameter("common.feature_names").as_string_array();
  params.feature_indices = node.get_parameter("common.feature_indices").as_integer_array();
  if (params.feature_names.size() != params.feature_indices.size()) {
    RCLCPP_WARN(
      node.get_logger(),
      "Parameter common.feature_names (%zu) and common.feature_indices (%zu) size mismatch; using defaults",
      params.feature_names.size(),
      params.feature_indices.size());
    params.feature_names = kDefaultFeatureNames;
    params.feature_indices = kDefaultFeatureIndices;
  }

  node.get_parameter("debug_flag", params.debug_flag);
  node.get_parameter("default_task", params.default_task);
  node.get_parameter("control_frequency_hz", params.control_frequency_hz);
  node.get_parameter("default_speed", params.default_speed);
  node.get_parameter("max_turn_speed", params.max_turn_speed);
  node.get_parameter("aggressive_turn_angle_deg", params.aggressive_turn_angle_rad);
  params.aggressive_turn_angle_rad *= kDegToRad;
  node.get_parameter("kp_turn", params.kp_turn);
  node.get_parameter("kd_turn", params.kd_turn);
  node.get_parameter("fps_log_period_s", params.fps_log_period_s);
  node.get_parameter("servo_90_pwm_diff", params.servo_90_pwm_diff);
  node.get_parameter("gripper_linear_speed", params.gripper_linear_speed);
  node.get_parameter("gripper_yaw_speed", params.gripper_yaw_speed);
  node.get_parameter("servo_sleep", params.servo_sleep);
  node.get_parameter("pickup_sleep", params.pickup_sleep);
  node.get_parameter("stop_cooldown_s", params.stop_cooldown_s);
  node.get_parameter("task.target_gain", params.task_config.target_gain);
  node.get_parameter("task.repellant_gain", params.task_config.repellant_gain);
  node.get_parameter("task.repellant_range", params.task_config.repellant_range);
  node.get_parameter("task.repellant_ellipse_x", params.task_config.repellant_ellipse_x);
  node.get_parameter("task.repellant_ellipse_y", params.task_config.repellant_ellipse_y);
  node.get_parameter("task.repellant_passed_margin_rad", params.task_config.repellant_passed_margin_rad);
  node.get_parameter("gripper.task_timeout_s", params.gripper_config.task_timeout_s);
  node.get_parameter("gripper.drop_angle_deadband_deg", params.gripper_config.drop_angle_deadband_deg);
  node.get_parameter("gripper.pickup_angle_deadband_deg", params.gripper_config.pickup_angle_deadband_deg);
  node.get_parameter("flare.scan_target_reached_threshold_m", params.flare_config.scan_target_reached_threshold_m);
  node.get_parameter("flare.target_reached_threshold_m", params.flare_config.target_reached_threshold_m);
  node.get_parameter("aruco.target_reached_threshold_m", params.aruco_config.target_reached_threshold_m);
  node.get_parameter("buckets.target_reached_threshold_m", params.buckets_config.target_reached_threshold_m);
  node.get_parameter("buckets.lock_bucket_proximity_threshold_m", params.buckets_config.lock_bucket_proximity_threshold_m);
  node.get_parameter("gate.ellipse_x", params.gate_config.ellipse_x);
  node.get_parameter("gate.ellipse_y", params.gate_config.ellipse_y);
  node.get_parameter("gate.forward_exit_margin_m", params.gate_config.forward_exit_margin_m);
  node.get_parameter("gate.apf_target_x_offset_m", params.gate_config.apf_target_x_offset_m);
  node.get_parameter("gate.gate_gain", params.gate_config.gate_gain);
  node.get_parameter("gate.blind_gate_gain_factor", params.gate_config.blind_gate_gain_factor);
  node.get_parameter("gate.repellant_gain", params.gate_config.repellant_gain);
  node.get_parameter("gate.repellant_range", params.gate_config.repellant_range);
  node.get_parameter("gate.repellant_ellipse_x", params.gate_config.repellant_ellipse_x);
  node.get_parameter("gate.repellant_ellipse_y", params.gate_config.repellant_ellipse_y);
  node.get_parameter("gate.repellant_passed_margin_m", params.gate_config.repellant_passed_margin_m);
  node.get_parameter("qual_gate.angle_tolerance_deg", params.qual_gate_config.angle_tolerance_rad);
  params.qual_gate_config.angle_tolerance_rad *= kDegToRad;
  node.get_parameter("qual_gate.slam_trust", params.qual_gate_config.slam_trust);
  node.get_parameter("qual_gate.ellipse_x", params.qual_gate_config.ellipse_x);
  node.get_parameter("qual_gate.ellipse_y", params.qual_gate_config.ellipse_y);
  node.get_parameter("qual_gate.forward_exit_margin_m", params.qual_gate_config.forward_exit_margin_m);
  node.get_parameter("qual_gate.backward_exit_margin_m", params.qual_gate_config.backward_exit_margin_m);
  node.get_parameter("qual_gate.turn_target_yaw_offset_deg", params.qual_gate_config.turn_target_yaw_offset_rad);
  params.qual_gate_config.turn_target_yaw_offset_rad *= kDegToRad;
  node.get_parameter("qual_gate.apf_target_x_offset_m", params.qual_gate_config.apf_target_x_offset_m);

  return params;
}
