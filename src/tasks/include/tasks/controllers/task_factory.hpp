#ifndef TASK_FACTORY_HPP
#define TASK_FACTORY_HPP

#include <memory>
#include <vector>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>

#include "tasks/task.hpp"
#include "tasks/task_config.hpp"


struct GateConfig {
  double ellipse_x = 5.0;
  double ellipse_y = 0.75;
  double forward_exit_margin_m = 0.8;
  double apf_target_x_offset_m = -1.0;
  double gate_gain = 5.0;
  double blind_gate_gain_factor = 0.5;
  double repellant_gain = 3.0;
  double repellant_range = 2.0;
  double repellant_ellipse_x = 1.5;
  double repellant_ellipse_y = 1.0;
  double repellant_passed_margin_m = 0.5;
  std::vector<std::string> repellant_names = {"flare_1", "flare_2", "flare_3"};
};

struct QualGateConfig {
  double angle_tolerance_rad = 1.5 * M_PI / 180.0;
  double slam_trust = 0.7;
  double ellipse_x = 5.0;
  double ellipse_y = 0.75;
  double forward_exit_margin_m = 0.8;
  double backward_exit_margin_m = 0.8;
  double turn_target_yaw_offset_rad = M_PI - 0.001;
  double apf_target_x_offset_m = -1.0;
};

struct FlareConfig {
  double scan_target_reached_threshold_m = 2.0;
  double target_reached_threshold_m = 0.1;
};

struct ArucoConfig {
  double target_reached_threshold_m = 0.5;
};

struct BucketsConfig {
  double target_reached_threshold_m = 0.5;
  double lock_bucket_proximity_threshold_m = 0.5;
  std::vector<std::string> repellant_names = {"flare_1", "flare_2", "flare_3", "gate_left", "gate_right"};
};

struct GripperConfig {
  double task_timeout_s = 20.0;
  double drop_angle_deadband_deg = 30.0;
  double pickup_angle_deadband_deg = 10.0;
};

class Gripper : public Task {
public:
  explicit Gripper(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const GripperConfig & config = GripperConfig(),
    const rclcpp::Time start_time = rclcpp::Time(),
    bool drop = true);
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;

private:
  bool drop_;
  rclcpp::Time timeout_;
  GripperConfig config_;
};

class Aruco : public Task {
public:
  explicit Aruco(std::shared_ptr<tf2_ros::Buffer> tf_buffer, const ArucoConfig & config = ArucoConfig());
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;
private:
  ArucoConfig config_;
};

class Buckets : public Task {
public:
  explicit Buckets(std::shared_ptr<tf2_ros::Buffer> tf_buffer, const BucketsConfig & config = BucketsConfig(), bool drop = true);
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;

private:
  bool drop_;
  BucketsConfig config_;
};

class Flare : public Task {
public:
  explicit Flare(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const FlareConfig & config = FlareConfig(),
    bool scan = true);
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;

private:
  int target_flare_ = 0;
  std::string target_sequence_[3] = {"flare_1", "flare_2", "flare_3"};
  FlareConfig config_;
  bool scan_;
  
};

class Gate : public Task {
public:
  explicit Gate(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const GateConfig & config = GateConfig(),
    bool forward = true);
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;

private:
  GateConfig config_;
  bool forward_;
  Task::Pos calculateAPF(const Task::Pos& robot_pos, const Task::Pos& gate_left, const Task::Pos& gate_right, const std::vector<Task::Pos>& repellants);
};

class QualGate : public Task {
public:
  explicit QualGate(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const QualGateConfig & config = QualGateConfig(),
    bool forward = true);
  int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) override;

private:
  enum State {
    FORWARD,
    TURN,
    BACKWARD
  };

  State state_ = FORWARD;
  QualGateConfig config_;

  Task::Pos calculateAPF(const Task::Pos& robot_pos, const Task::Pos& gate_left, const Task::Pos& gate_right);
};

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
  bool initial);

#endif
