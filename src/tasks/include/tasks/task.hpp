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
    double yaw=0; //not part of arithmetic
    
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
   * @brief Get flare transform with validation that the flare is within map bounds.
   * Checks that flare is between flag and gate in x-direction and within [-7.0, 7.0] in y.
   * If validation fails, returns a transform with x = -100.0 as an error signal.
   * @param flare_name Name of the flare frame (e.g., "flare_1")
   * @param from Reference frame (default "map")
   * @return Transform with -100 in x if validation fails, otherwise the valid flare transform
   */
  geometry_msgs::msg::TransformStamped get_flare_transform(const std::string &flare_name, const std::string &from="map");

  static double normalize_angle(double a);
  Pos normalize_pos(const Pos& p);
  Pos clean_command(Pos command, double robot_yaw);

  /**
   * @brief Get the angle formed at point b by the line segments ab and bc (abc colinear being 0)
   * Anticlockwise angles are positive, clockwise angles are negative
   * @param a 
   * @param b 
   * @param c 
   * @return double 
   */
  double getAngle(const Task::Pos a, const Task::Pos b, const Task::Pos c);

  /**
   * @brief Calculate elliptical field force around an object
   * @param robot_pos Current robot position
   * @param object_pos Object position (center of ellipse)
   * @param counter_clockwise True for CCW field, false for CW field
   * @param x_radius Radius of ellipse along x-axis
   * @param y_radius Radius of ellipse along y-axis
   * @param range Distance at which the force magnitude is 0.0, 0 if infinite
   * @return Pure tangential force vector
   */
  Pos calculateEllipticalField(const Pos& robot_pos, const Pos& object_pos, bool counter_clockwise, double x_radius, double y_radius, double range=0.0);

  /**
   * @brief Calculate standard Artificial Potential Field (APF) force with attractive target and circular repellant
   * @param robot_pos Current robot position
   * @param target_pos 
   * @param repellants force direction determined by what side of the line of target it is on
   * @return Pos 
   */
  Pos calculateStandardAPF(const Task::Pos& robot_pos, const Task::Pos& target_pos, const std::vector<Task::Pos>& repellants);
};
