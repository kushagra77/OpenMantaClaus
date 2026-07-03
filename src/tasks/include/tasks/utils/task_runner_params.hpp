#ifndef TASKS__UTILS__TASK_RUNNER_PARAMS_HPP_
#define TASKS__UTILS__TASK_RUNNER_PARAMS_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "tasks/task_config.hpp"
#include "tasks/controllers/task_factory.hpp"

struct TaskRunnerParams {
  std::vector<std::string> feature_names;
  std::vector<int64_t> feature_indices;
  bool debug_flag = true;
  std::string default_task = "gate";
  double control_frequency_hz = 20.0;
  double default_speed = 0.3;
  double max_turn_speed = 0.5;
  double aggressive_turn_angle_rad = M_PI / 4.0;
  double kp_turn = 1.0 / M_PI;
  double kd_turn = 0.1;
  double fps_log_period_s = 4.0;
  int servo_90_pwm_diff = 400;
  double gripper_linear_speed = 1.0;
  double gripper_yaw_speed = 1.0;
  double servo_sleep = 1.0;
  double pickup_sleep = 3.0;
  TaskConfig task_config;
  FlareConfig flare_config;
  ArucoConfig aruco_config;
  BucketsConfig buckets_config;
  GripperConfig gripper_config;
  GateConfig gate_config;
  QualGateConfig qual_gate_config;
};

TaskRunnerParams load_task_runner_params(rclcpp::Node & node);

#endif