#include "tasks/controllers/task_factory.hpp"

#include <utility>

Flare::Flare(
  std::shared_ptr<tf2_ros::Buffer> tf_buffer,
  const FlareConfig & config,
  bool scan)
: Task(std::move(tf_buffer)), config_(config), scan_(scan) {
  if (!scan_) {
    // if not sccanning, assign target sequence
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (Task::flare_colors_[i] == Task::flare_order_[j]) {
          target_sequence_[j] = "flare_" + std::to_string(i + 1);
          break;
        }
      }
    }
  }
  
}

int Flare::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;

  // ASSUMPTION: The try-catch below acts as an initial guard. The subsequent lookups on
  // lines outside the try-catch assume that EKF SLAM is actively running and publishing
  // these frames (base_link, flare_1, flag, gate_left, gate_right) at all times, making them safe to access.
  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_flare_transform("flare_1");

  } catch(const tf2::TransformException &ex) {
    return -10; // just keep doing what we were doing, hopefully will get the transform in the next iteration
  }

  // check scan and lock for early return
  if (scan_ and Task::lock_flares_) {
    return -1; // no need to keep scanning
  }
  
  // if target_flare_ is 0, just starting, assigning target flare
  if (target_flare_ == 0) {
    target_flare_ = 1;
  }

  // get all relevant transforms
  auto robot_transform = get_transform("base_link");
  
  // Get target flare transform with validation
  auto target_transform = get_flare_transform(target_sequence_[target_flare_ - 1]);
  
  // Check if target flare is out of bounds (x == -100)
  if (target_transform.transform.translation.x <= 0.0) {
    // Flare is out of bounds, consider it reached and move to next flare
    target_flare_++;
    
    // check if task complete
    if (target_flare_ > 3) {
      // if scan complete, lock in the flares
      if (scan_) Task::lock_in_flares();
      return -1; // task complete
    }
    return target_flare_;
  }
  
  auto repellant_flare_1 = target_sequence_[(target_flare_ % 3)]; // the other flare that is not target
  auto repellant_flare_2 = target_sequence_[(target_flare_ + 1)%3]; // the other flare that is not target
  
  
  // ------ repellant logic -------
  std::vector<Task::Pos> repellants;
  auto flag_tf = get_transform("flag");
  repellants.push_back({
    flag_tf.transform.translation.x,
    flag_tf.transform.translation.y
  });
  auto gate_left_tf = get_transform("gate_left");
  repellants.push_back({
    gate_left_tf.transform.translation.x,
    gate_left_tf.transform.translation.y
  });
  auto gate_right_tf = get_transform("gate_right");
  repellants.push_back({
    gate_right_tf.transform.translation.x,
    gate_right_tf.transform.translation.y
  });

  if (Task::feature_seen[repellant_flare_1]) {
    try {
      auto repellant_tf = get_flare_transform(repellant_flare_1);
      // Only add to repellants if the flare is within bounds (x != -100)
      if (repellant_tf.transform.translation.x >= 0.0) {
        repellants.push_back({
          repellant_tf.transform.translation.x,
          repellant_tf.transform.translation.y
        });
      }
    } catch (const tf2::TransformException &ex) {
      // Ignore if we can't get the repellant flare transform
    }
  }

  if (Task::feature_seen[repellant_flare_2]) {
    try {
      auto repellant_tf = get_flare_transform(repellant_flare_2);
      // Only add to repellants if the flare is within bounds (x != -100)
      if (repellant_tf.transform.translation.x >= 0.0) {
        repellants.push_back({
          repellant_tf.transform.translation.x,
          repellant_tf.transform.translation.y
        });
      }
    } catch (const tf2::TransformException &ex) {
      // Ignore if we can't get the repellant flare transform
    }
  }
  // ----------------------------

  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };

  const Task::Pos target_p = {
    target_transform.transform.translation.x,
    target_transform.transform.translation.y
  };

  // calculate final command
  command_ = clean_command(calculateStandardAPF(robot_p, target_p, repellants), robot_p.yaw);

  // check if target reached, increment target idx and return state
  auto error = target_p - robot_p;

  // check if reached target
  if (scan_ and error.norm() < config_.scan_target_reached_threshold_m) {
    target_flare_++;
  } else if (error.norm() < config_.target_reached_threshold_m) {
    target_flare_++;
  }

  // check if task complete
  if (target_flare_ > 3) {
    // if scan complete, lock in the flares
    if (scan_) Task::lock_in_flares();
    return -1; // task complete
  }
  return target_flare_;
}
