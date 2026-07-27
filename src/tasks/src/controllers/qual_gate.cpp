#include "tasks/controllers/task_factory.hpp"
#include "tasks/task_protocol.hpp"

#include <algorithm>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <utility>

QualGate::QualGate(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const QualGateConfig & config,
    bool forward)
    : Task(std::move(tf_buffer)), config_(config) {
    state_ = forward ? FORWARD : BACKWARD;
}

int QualGate::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;
  
  // ASSUMPTION: The try-catch below acts as an initial guard. The subsequent lookups on
  // lines outside the try-catch assume that EKF SLAM is actively running and publishing
  // these frames (base_link, qual_gate_left, qual_gate_right) at all times, making them safe to access.
  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_transform("qual_gate_left");

  } catch(const tf2::TransformException &ex) {
    return task_protocol::kTransformUnavailable;
  }

  auto robot_transform = get_transform("base_link");
  auto gate_l_transform = get_transform("qual_gate_left");
  auto gate_r_transform = get_transform("qual_gate_right");
  if (state_ == BACKWARD) {
    std::swap(gate_l_transform, gate_r_transform);
  }

  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };
  const Task::Pos gate_l_p = {
    gate_l_transform.transform.translation.x,
    gate_l_transform.transform.translation.y
  };
  const Task::Pos gate_r_p = {
    gate_r_transform.transform.translation.x,
    gate_r_transform.transform.translation.y
  };

  command_ = {0.0, 0.0, 0.0};
  switch (state_) {
    case FORWARD:
      if (robot_p.x > gate_l_p.x + config_.forward_exit_margin_m) {
        state_ = TURN;
        target_yaw_ = normalize_angle(robot_p.yaw + config_.turn_target_yaw_offset_rad);
      } else {
        command_ = calculateAPF(robot_p, gate_l_p, gate_r_p);
      }
      break;
    case TURN:
      if (std::abs(normalize_angle(target_yaw_ - robot_p.yaw)) < config_.angle_tolerance_rad) {
        state_ = BACKWARD;
      } else {
        command_.yaw = normalize_angle(target_yaw_ - robot_p.yaw);
        return static_cast<int>(state_);
      }
      break;
    case BACKWARD:
      if (robot_p.x < gate_l_p.x - config_.backward_exit_margin_m) {
          return task_protocol::kTaskComplete;
      }
      command_ = calculateAPF(robot_p, gate_l_p, gate_r_p);
      break;
  }
  
  command_ = clean_command(command_, robot_p.yaw);

  return static_cast<int>(state_);
}

Task::Pos QualGate::calculateAPF(const Task::Pos& robot_pos, const Task::Pos& gate_left, const Task::Pos& gate_right) {
  if (robot_pos.y > std::max(gate_left.y, gate_right.y)
      || robot_pos.y < std::min(gate_left.y, gate_right.y)) {
    Task::Pos target = ((gate_left + gate_right) * 0.5);
    target.x -= config_.apf_target_x_offset_m;
    return normalize_pos({target - robot_pos});
  }

  const Task::Pos left_force = calculateEllipticalField(robot_pos, gate_left, true, config_.ellipse_x, config_.ellipse_y);
  const Task::Pos right_force = calculateEllipticalField(robot_pos, gate_right, false, config_.ellipse_x, config_.ellipse_y);
  return normalize_pos(left_force + right_force);
}
