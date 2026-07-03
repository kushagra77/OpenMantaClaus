#include "brain/utils/brain_params.hpp"

BrainMissionConfig load_brain_mission_config(rclcpp::Node & node)
{
  BrainMissionConfig config;

  node.declare_parameter("mission.main_run", config.main_run);
  node.declare_parameter<std::vector<std::string>>("mission.main_sequence", config.main_sequence);
  node.declare_parameter<std::vector<std::string>>("mission.qual_sequence", config.qual_sequence);
  node.declare_parameter("setup.target_depth_m", config.setup_target_depth_m);
  node.declare_parameter("setup.start_delay_s", config.setup_start_delay_s);
  node.declare_parameter("setup.setpoint_publish_count", config.setpoint_publish_count);
  node.declare_parameter("setup.setpoint_publish_interval_ms", config.setpoint_publish_interval_ms);

  node.get_parameter("mission.main_run", config.main_run);
  node.get_parameter("mission.main_sequence", config.main_sequence);
  node.get_parameter("mission.qual_sequence", config.qual_sequence);
  node.get_parameter("setup.target_depth_m", config.setup_target_depth_m);
  node.get_parameter("setup.start_delay_s", config.setup_start_delay_s);
  node.get_parameter("setup.setpoint_publish_count", config.setpoint_publish_count);
  node.get_parameter("setup.setpoint_publish_interval_ms", config.setpoint_publish_interval_ms);

  return config;
}

std::queue<std::string> build_task_queue(const std::vector<std::string> & sequence)
{
  std::queue<std::string> queue;
  for (const auto & task : sequence) {
    queue.push(task);
  }
  return queue;
}
