#include "tasks/controllers/task_factory.hpp"


std::unique_ptr<Task> create_task_executor(
  const std::string & task_name,
  const std::shared_ptr<tf2_ros::Buffer> & tf_buffer,
  const TaskConfig & task_config,
  const FlareConfig & flare_config,
  const ArucoConfig & aruco_config,
  const BucketsConfig & buckets_config,
  const GripperConfig & gripper_config,
  const GateConfig & gate_config,
  const QualGateConfig & qual_gate_config,
  const rclcpp::Time start_time,
  bool initial)
{
  std::unique_ptr<Task> executor;
  if (task_name == "qual_gate") {
    executor = std::make_unique<QualGate>(tf_buffer, qual_gate_config, initial);
  } else if (task_name == "gate") {
    executor = std::make_unique<Gate>(tf_buffer, gate_config, initial);
  } else if (task_name == "gripper") {
    executor = std::make_unique<Gripper>(tf_buffer, gripper_config, start_time, initial);
  } else if (task_name == "bucket") {
    executor = std::make_unique<Buckets>(tf_buffer, buckets_config, initial);
  } else if (task_name == "flare") {
    executor = std::make_unique<Flare>(tf_buffer, flare_config, initial);
  } else if (task_name == "aruco") {
    executor = std::make_unique<Aruco>(tf_buffer, aruco_config);
  } 
  
  if (executor) {
    executor->set_task_config(task_config);
  }
  
  return executor;
}
