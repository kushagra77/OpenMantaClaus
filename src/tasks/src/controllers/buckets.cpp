#include "tasks/controllers/task_factory.hpp"

#include <utility>

Buckets::Buckets(std::shared_ptr<tf2_ros::Buffer> tf_buffer, const BucketsConfig & config, bool drop)
  : Task(std::move(tf_buffer)), config_(config), drop_(drop) {}

int Buckets::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;

  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_transform("bucket_1");

  } catch(const tf2::TransformException &ex) {
    return -10; // stop all rc commands and wait until transform is found
  }

  auto robot_transform = get_transform("base_link");
  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };

  // figure out target
  int target_id = 2; // default to bucket 2 if can't find blue bucket
  if (Task::lock_buckets_) {
    // if buckets locked, target is the locked colour
    for (int i = 0; i < 4; i++) {
      if (bucket_colors_[i] == BLUE) {
        target_id = i + 1;
        break;
      }
    }
  }
  auto target_transform = get_transform("bucket_" + std::to_string(target_id));
  Task::Pos target_p = {
    target_transform.transform.translation.x,
    target_transform.transform.translation.y
  };

  double dist = (target_p - robot_p).norm();
  // check deadbands, if target within target_reached_threshold_m, then return -1 for completion
  if (dist < config_.target_reached_threshold_m) {
    return -1;
  } else if (drop_ && dist < config_.lock_bucket_proximity_threshold_m && !Task::lock_buckets_) {
    // if dropping ball, and target within lock_bucket_proximity_threshold_m lock in the buckets.
    lock_in_buckets();
  }
  
  // if dropping ball, no repellants, if picking up ball, gates and flare repelllants
  std::vector<Task::Pos> repellants;
  if (!drop_) {
    std::vector<std::string> repellant_names = {"flare_1", "flare_2", "flare_3", "gate_left", "gate_right"};
    for (auto repellant_name : repellant_names) {
      if (Task::feature_seen[repellant_name]) {
        auto repellant_tf = get_transform(repellant_name);
        if (repellant_name[0] == 'f') {
          try {
            repellant_tf = get_flare_transform(repellant_name);
            // Only add to repellants if the flare is within bounds (x != -100)
            if (repellant_tf.transform.translation.x < 0.0) {
              continue;
            }
          } catch (const tf2::TransformException &ex) {
            // Ignore if we can't get the repellant flare transform
          }
        }
        repellants.push_back({
          repellant_tf.transform.translation.x,
          repellant_tf.transform.translation.y
        });
      }
    }
  }

  command_ = clean_command(calculateStandardAPF(robot_p, target_p, repellants), robot_p.yaw);
  return 1;
}
