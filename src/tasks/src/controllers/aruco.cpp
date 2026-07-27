#include "tasks/controllers/task_factory.hpp"
#include "tasks/task_protocol.hpp"

#include <utility>

Aruco::Aruco(std::shared_ptr<tf2_ros::Buffer> tf_buffer, const ArucoConfig & config)
  : Task(std::move(tf_buffer)), config_(config) {}

int Aruco::execute(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  (void)msg;

  // ASSUMPTION: The try-catch below acts as an initial guard. The subsequent lookups on
  // lines outside the try-catch assume that EKF SLAM is actively running and publishing
  // these frames (base_link, aruco_marker, flag) at all times, making them safe to access.
  try {
    auto robot_transform = get_transform("base_link");
    auto gate_l_transform = get_transform("aruco_marker");

  } catch(const tf2::TransformException &ex) {
    return task_protocol::kTransformUnavailable;
  }

  if (Task::flare_order_[0] != -1) return task_protocol::kTaskComplete;

  auto robot_transform = get_transform("base_link");
  auto aruco_transform = get_transform("aruco_marker");
  auto flag_transform = get_transform("flag");

  const Task::Pos robot_p = {
    robot_transform.transform.translation.x,
    robot_transform.transform.translation.y,
    tf2::getYaw(robot_transform.transform.rotation)
  };

  const Task::Pos aruco_p = {
    aruco_transform.transform.translation.x,
    aruco_transform.transform.translation.y
  };

  double dist = (aruco_p - robot_p).norm();
  if (dist < config_.target_reached_threshold_m) {
    return task_protocol::kTaskComplete;
  }

  const Task::Pos flag_p = {
    flag_transform.transform.translation.x,
    flag_transform.transform.translation.y
  };

  std::vector<Task::Pos> repellants;
  repellants.push_back(flag_p);
  command_ = clean_command(calculateStandardAPF(robot_p, aruco_p, repellants), robot_p.yaw);

  return 1;
}
