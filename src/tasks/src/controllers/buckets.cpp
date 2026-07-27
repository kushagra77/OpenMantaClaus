#include "tasks/controllers/task_factory.hpp"
#include "tasks/task_protocol.hpp"

#include <utility>

Buckets::Buckets(std::shared_ptr<tf2_ros::Buffer> tf_buffer, const BucketsConfig & config, bool drop)
  : Task(std::move(tf_buffer)), config_(config), drop_(drop) {}

int Buckets::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;

  // ASSUMPTION: The try-catch below acts as an initial guard. The subsequent lookups on
  // lines outside the try-catch assume that EKF SLAM is actively running and publishing
  // these frames (base_link, bucket_1, target bucket, repellants) at all times, making them safe to access.
  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_transform("bucket_1");

  } catch(const tf2::TransformException &ex) {
    return task_protocol::kTransformUnavailable;
  }

  auto robot_transform = get_transform("base_link");
  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };

  int target_id = 2;
  if (Task::lock_buckets_) {
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
  if (dist < config_.target_reached_threshold_m) {
    return task_protocol::kTaskComplete;
  } else if (drop_ && dist < config_.lock_bucket_proximity_threshold_m && !Task::lock_buckets_) {
    lock_in_buckets();
  }
  
  std::vector<Task::Pos> repellants;
  if (!drop_) {
    std::vector<std::string> repellant_names = config_.repellant_names;
    for (auto repellant_name : repellant_names) {
      if (Task::feature_seen[repellant_name]) {
        auto repellant_tf = get_transform(repellant_name);
        if (repellant_name[0] == 'f') {
          try {
            repellant_tf = get_flare_transform(repellant_name);
            if (repellant_tf.transform.translation.x < 0.0) {
              continue;
            }
          } catch (const tf2::TransformException &ex) {
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
