#include "tasks/controllers/task_factory.hpp"
#include "tasks/task_protocol.hpp"

#include <algorithm>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <utility>

Gate::Gate(
    std::shared_ptr<tf2_ros::Buffer> tf_buffer,
    const GateConfig & config,
    bool forward)
    : Task(std::move(tf_buffer)), config_(config), forward_(forward) {}

int Gate::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;

  // ASSUMPTION: The try-catch below acts as an initial guard. The subsequent lookups on
  // lines outside the try-catch assume that EKF SLAM is actively running and publishing
  // these frames (base_link, gate_left, gate_right) at all times, making them safe to access.
  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_transform("gate_left");

  } catch(const tf2::TransformException &ex) {
    return task_protocol::kTransformUnavailable;
  }

  auto robot_transform = get_transform("base_link");
  auto gate_l_transform = get_transform("gate_left");
  auto gate_r_transform = get_transform("gate_right");
  
  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };
  Task::Pos gate_l_p = {
    gate_l_transform.transform.translation.x,
    gate_l_transform.transform.translation.y
  };
  Task::Pos gate_r_p = {
    gate_r_transform.transform.translation.x,
    gate_r_transform.transform.translation.y
  };
  
  std::vector<Task::Pos> repellants;
  std::vector<std::string> repellant_names = config_.repellant_names;
  

  if (!forward_) {
    if (robot_p.x < gate_l_p.x - config_.forward_exit_margin_m) {
      return task_protocol::kTaskComplete;
    }
    std::swap(gate_l_p, gate_r_p);
    repellant_names.clear();
  } else {
    if (robot_p.x > gate_l_p.x + config_.forward_exit_margin_m) {
      return task_protocol::kTaskComplete;
    }
    // ASSUMPTION: The 'flag' is a default/essential repellant that must always be present.
    // The lookup assumes that the 'flag' TF frame is actively published by EKF SLAM.
    auto flag_tf = get_transform("flag");
    repellants.push_back({
      flag_tf.transform.translation.x,
      flag_tf.transform.translation.y
    });
  }
  


  for (auto repellant_name : repellant_names) {
    if (Task::feature_seen[repellant_name]) {
      try {
        auto repellant_tf = get_flare_transform(repellant_name);
        if (repellant_tf.transform.translation.x >= 0.0) {
          repellants.push_back({
            repellant_tf.transform.translation.x,
            repellant_tf.transform.translation.y
          });
        }
      } catch (const tf2::TransformException &ex) {
      }
    }
  }


  command_ = clean_command(calculateAPF(robot_p, gate_l_p, gate_r_p, repellants), robot_p.yaw);
  return 1;
}

Task::Pos Gate::calculateAPF(const Task::Pos& robot_pos, const Task::Pos& gate_left, const Task::Pos& gate_right, const std::vector<Task::Pos>& repellants) {
  
  Task::Pos sum_force{0.0, 0.0};
  double gate_gain = config_.gate_gain;
  const double repellant_gain = config_.repellant_gain;
  const double repellant_range = config_.repellant_range;
  const double repellant_ellipse_x = config_.repellant_ellipse_x;
  const double repellant_ellipse_y = config_.repellant_ellipse_y;
  const double repellant_passed_margin_m = config_.repellant_passed_margin_m;
  if (!(Task::feature_seen["gate_left"] || Task::feature_seen["gate_right"])) {
    gate_gain *= config_.blind_gate_gain_factor;
  }

  if (robot_pos.y > std::max(gate_left.y, gate_right.y)
      || robot_pos.y < std::min(gate_left.y, gate_right.y)) {
    Task::Pos target = (gate_left + gate_right) * 0.5;
    target.x += config_.apf_target_x_offset_m;
    sum_force += (target - robot_pos) * (2.0 * gate_gain);
  } else {
    const Task::Pos left_force = calculateEllipticalField(robot_pos, gate_left, true, config_.ellipse_x, config_.ellipse_y);
    const Task::Pos right_force = calculateEllipticalField(robot_pos, gate_right, false, config_.ellipse_x, config_.ellipse_y);
    sum_force += (left_force + right_force) * gate_gain;
  }

  for (auto repellant: repellants) {
    if (repellant.x < robot_pos.x - repellant_passed_margin_m) {
      continue;
    }
    bool counter_clockwise = (repellant.y >= (gate_left.y + gate_right.y) / 2.0);
    if (!forward_) counter_clockwise = !counter_clockwise;
    sum_force += calculateEllipticalField(robot_pos, repellant, counter_clockwise, repellant_ellipse_x, repellant_ellipse_y, repellant_range) * repellant_gain;
  }


  return normalize_pos(sum_force);
}
