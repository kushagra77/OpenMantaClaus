#pragma once
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/buffer.h>
#include <interfaces/msg/feature_observations.hpp>
#include <tf2/utils.h>
#include <tasks/task_config.hpp>

#include <cmath>
#include <string>
#include <set>
#include <unordered_map>

#define RED 1
#define BLUE 2
#define YELLOW 3
#define UNKNOWN 0

class Task {
public:
  explicit Task(std::shared_ptr<tf2_ros::Buffer> tf_buffer);
  static std::unordered_map<std::string, bool> feature_seen;
  inline static bool lock_flares_ = false;
  inline static bool lock_buckets_ = false;
  inline static int flare_colors_[3] = {UNKNOWN, UNKNOWN, UNKNOWN};
  inline static int bucket_colors_[4] = {UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN};
  inline static int flare_order_[3] = {-1, -1, -1};
  static void process_colored_feature(int feature_id, int color);
  static void lock_in_flares();
  static void lock_in_buckets();
  virtual int execute(const interfaces::msg::FeatureObservations::SharedPtr msg) = 0;
  struct Pos {
    double x;
    double y;
    double yaw = 0;
    
    Pos operator*(double scalar) const;
    Pos operator+(const Pos& other) const;
    Pos operator-(const Pos& other) const;
    Pos operator/(double scalar) const;
    Pos& operator+=(const Pos& other);
    double norm() const;
  };

  Pos getCommand() const;

  void set_task_config(const TaskConfig& config) {
    task_config_ = config;
  }

protected:

  Pos command_;
  double target_yaw_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  TaskConfig task_config_;

  geometry_msgs::msg::TransformStamped get_transform(const std::string &to, const std::string &from="map");

  /**
   * @brief Get a flare transform and validate its map bounds.
   * Returns x = -100.0 when the flare is outside the expected course region.
   */
  geometry_msgs::msg::TransformStamped get_flare_transform(const std::string &flare_name, const std::string &from="map");

  static double normalize_angle(double a);
  Pos normalize_pos(const Pos& p);
  Pos clean_command(Pos command, double robot_yaw);

  /**
   * @brief Get the signed angle at point b for the polyline a->b->c.
   */
  double getAngle(const Task::Pos a, const Task::Pos b, const Task::Pos c);

  /**
   * @brief Calculate a tangential elliptical repulsion field around an object.
   */
  Pos calculateEllipticalField(const Pos& robot_pos, const Pos& object_pos, bool counter_clockwise, double x_radius, double y_radius, double range=0.0);

  /**
   * @brief Calculate the standard APF force for a target with optional repellants.
   */
  Pos calculateStandardAPF(const Task::Pos& robot_pos, const Task::Pos& target_pos, const std::vector<Task::Pos>& repellants);
};
