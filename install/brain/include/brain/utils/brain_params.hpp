#ifndef BRAIN__UTILS__BRAIN_PARAMS_HPP_
#define BRAIN__UTILS__BRAIN_PARAMS_HPP_

#include <queue>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"

struct BrainMissionConfig {
  bool main_run = true;
  std::vector<std::string> main_sequence = {"gate", "bucket", "-gate", "flare", "aruco", "-flare", "-bucket"};
  std::vector<std::string> qual_sequence = {"qual_gate", "-qual_gate"};
  double setup_target_depth_m = 0.8;
  int setup_start_delay_s = 15;
  int setpoint_publish_count = 5;
  int setpoint_publish_interval_ms = 50;
};

BrainMissionConfig load_brain_mission_config(rclcpp::Node & node);
std::queue<std::string> build_task_queue(const std::vector<std::string> & sequence);

#endif