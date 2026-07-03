#include "tasks/task.hpp"

#include <stdexcept>
#include <utility>

std::unordered_map<std::string, bool> Task::feature_seen;

Task::Task(std::shared_ptr<tf2_ros::Buffer> tf_buffer) : tf_buffer_(std::move(tf_buffer)) {}

void Task::lock_in_flares() {
  if (lock_flares_) {
    return;
  }
  std::set<int> unknown_flares;
  std::set<int> unknown_colors = {RED, BLUE, YELLOW};
  for (int i = 0; i < 3; i++) {
    if (flare_colors_[i] != UNKNOWN) {
      unknown_colors.erase(flare_colors_[i]);
    } else {
      unknown_flares.insert(i);
    }
  }
  if (unknown_flares.size() == 1) {
    int flare_idx = *unknown_flares.begin();
    int color = *unknown_colors.begin();
    flare_colors_[flare_idx] = color;
  } else if (unknown_flares.size() == 3) {
    flare_colors_[0] = RED;
    flare_colors_[1] = BLUE;
    flare_colors_[2] = YELLOW;
  } else if (unknown_flares.size() == 2) {
    if (unknown_flares.count(1)) {
      if (unknown_colors.count(RED)) {
        flare_colors_[1] = RED;
      } else {
        flare_colors_[1] = YELLOW;
      }
      unknown_flares.erase(1);
      unknown_colors.erase(flare_colors_[1]);
      int other_flare_idx = *unknown_flares.begin();
      flare_colors_[other_flare_idx] = *unknown_colors.begin();
    } else {
      // otherwise just assign arbitrarily
      auto it = unknown_flares.begin();
      auto colorit = unknown_colors.begin();
      flare_colors_[*it] = *colorit;
      it++;colorit++;
      flare_colors_[*it] = *colorit;
    }
  }

  lock_flares_ = true;
}

void Task::lock_in_buckets() {
  if (lock_buckets_) {
    return;
  }
  lock_buckets_ = true;
  for (int i = 0; i < 4; i++) {
    if (bucket_colors_[i] == BLUE) {
      for (int j = 0; j < 4; j++) {
        if (bucket_colors_[j] == UNKNOWN) {
          bucket_colors_[j] = RED;
        }
      }
      return;
    }
  }

  bool blue_assigned = false;
  for (int j = 1; j < 5; j++) {
    int i = j % 4;
    if (bucket_colors_[i] == UNKNOWN) {
      if (!blue_assigned) {
        bucket_colors_[i] = BLUE;
        blue_assigned = true;
      } else {
        bucket_colors_[i] = RED;
      }
    }
  }
}

void Task::process_colored_feature(int feature_id, int color) {
    if (feature_id >= 3 && feature_id <= 5 and !lock_flares_) {
        int f_idx = feature_id - 3;

        for (int i = 0; i < 3; i++) {
            if (flare_colors_[i] == color) {
        flare_colors_[i] = UNKNOWN;
            }
        }
        flare_colors_[f_idx] = color;

        int unassigned_count = 0;
        int unassigned_idx = -1;
        for (int i = 0; i < 3; i++) {
            if (flare_colors_[i] == UNKNOWN) {
                unassigned_count++;
                unassigned_idx = i;
            } else if (flare_colors_[i] == BLUE) {
        return;
            }
        }

        if (unassigned_count == 1) {
            flare_colors_[unassigned_idx] = BLUE;
        }
    } else if (feature_id >= 6 && feature_id <= 9 and !lock_buckets_ and feature_seen["bucket_4"]) {
        int b_idx = feature_id - 6;

        bucket_colors_[b_idx] = color;

        int unassigned_count = 0;
        int unassigned_idx = -1;
        for (int i = 0; i < 4; i++) {
            if (bucket_colors_[i] == UNKNOWN) {
                unassigned_count++;
                unassigned_idx = i;
            } else if (bucket_colors_[i] == BLUE) {
                for (int i = 0; i < 4; i++) {
                    if (bucket_colors_[i] == UNKNOWN) {
                        bucket_colors_[i] = RED;
                    }
                }
                lock_buckets_ = true;
                return; 
            }
        }
        if (unassigned_count == 1) {
            bucket_colors_[unassigned_idx] = BLUE;
            lock_buckets_ = true;
        }

    } else if (feature_id == 10) {
        int perm_val = color;

        if (perm_val != -1) {
            static const int permutations[6][3] = {
        {1, 2, 3},
        {1, 3, 2},
        {2, 3, 1},
        {2, 1, 3},
        {3, 1, 2},
        {3, 2, 1},
            };

            for (int i = 0; i < 3; i++) {
                flare_order_[i] = permutations[perm_val][i];
            }
        }
    }
}

Task::Pos Task::Pos::operator*(double scalar) const {
  return {x * scalar, y * scalar, yaw};
}

Task::Pos Task::Pos::operator+(const Pos& other) const {
  return {x + other.x, y + other.y, yaw};
}

Task::Pos Task::Pos::operator-(const Pos& other) const {
  return {x - other.x, y - other.y, yaw};
}

Task::Pos Task::Pos::operator/(double scalar) const {
  return {x / scalar, y / scalar, yaw};
}

Task::Pos& Task::Pos::operator+=(const Pos& other) {
  x += other.x;
  y += other.y;
  return *this;
}

double Task::Pos::norm() const {
  return std::sqrt(x * x + y * y);
}

double Task::getAngle(const Task::Pos a, const Task::Pos b, const Task::Pos c) {
  // Vector AB (incoming)
  double ab_x = b.x - a.x;
  double ab_y = b.y - a.y;
  
  // Vector BC (outgoing)
  double bc_x = c.x - b.x;
  double bc_y = c.y - b.y;

  double dot_product = ab_x * bc_x + ab_y * bc_y;
  double cross_product = ab_x * bc_y - ab_y * bc_x;

  return std::atan2(cross_product, dot_product);
}

Task::Pos Task::getCommand() const {
  return command_;
}

geometry_msgs::msg::TransformStamped Task::get_transform(const std::string &to, const std::string &from) {
  return tf_buffer_->lookupTransform(from, to, tf2::TimePointZero, std::chrono::milliseconds(20));

}

geometry_msgs::msg::TransformStamped Task::get_flare_transform(const std::string &flare_name, const std::string &from) {
  // Get the flare transform
  auto flare_tf = get_transform(flare_name, from);
  double flare_x = flare_tf.transform.translation.x;
  double flare_y = flare_tf.transform.translation.y;

  // Get flag and gate bounds to determine valid x range
  auto flag_tf = get_transform("flag", from);
  auto gate_tf = get_transform("gate_left", from);  // Use gate_left for x reference
  
  double flag_x = flag_tf.transform.translation.x;
  double gate_x = gate_tf.transform.translation.x;
  
  // Check both conditions:
  // 1. Flare x is between flag and gate
  // 2. Flare y is in [-7.0, 7.0]
  if (flare_x > flag_x && flare_x < gate_x && flare_y >= -7.0 && flare_y <= 7.0) {
    // Valid flare, return the original transform
    return flare_tf;
  }

  // Validation failed: return a transform with x = -100 as an error signal
  auto error_tf = flare_tf;
  error_tf.transform.translation.x = -100.0;
  return error_tf;
}


double Task::normalize_angle(double a) {
  return std::atan2(std::sin(a), std::cos(a));
}

Task::Pos Task::clean_command(Pos command, double robot_yaw) {
  command = normalize_pos(command);
  if (command.x != 0.0 || command.y != 0.0) {
    command.yaw = normalize_angle(std::atan2(command.y, command.x) - robot_yaw);
  }
  return command;
}


Task::Pos Task::normalize_pos(const Pos& p) {
  const double norm = p.norm();
  if (norm < 0.001) {
    return {0.0, 0.0, p.yaw};
  }
  return {p.x / norm, p.y / norm, p.yaw};
}

Task::Pos Task::calculateEllipticalField(const Pos& robot_pos, const Pos& object_pos, bool counter_clockwise, double x_radius, double y_radius, double range) {
  const double dx = robot_pos.x - object_pos.x;
  const double dy = robot_pos.y - object_pos.y;
  const double distance = std::sqrt(dx * dx + dy * dy);

  if (distance < 0.2) { // if within 20cm, already hit it..
    return {0.0, 0.0};
  }

  const double grad_x = 2.0 * dx / (x_radius * x_radius);
  const double grad_y = 2.0 * dy / (y_radius * y_radius);
  const double grad_magnitude = std::sqrt(grad_x * grad_x + grad_y * grad_y);

  if (grad_magnitude < 0.001) {
    return {0.0, 0.0};
  }

  const double normal_x = grad_x / grad_magnitude;
  const double normal_y = grad_y / grad_magnitude;

  double tangent_x;
  double tangent_y;
  if (counter_clockwise) {
    tangent_x = -normal_y;
    tangent_y = normal_x;
  } else {
    tangent_x = normal_y;
    tangent_y = -normal_x;
  }

  double force_magnitude = 1.0 / distance;

  if (range != 0.0) {
    if (distance > range) {
      return {0.0, 0.0};
    }
    force_magnitude *= (1.0 - distance / range);
  }
  return {force_magnitude * tangent_x, force_magnitude * tangent_y};
}

Task::Pos Task::calculateStandardAPF(const Task::Pos& robot_pos, const Task::Pos& target_pos, const std::vector<Task::Pos>& repellants) {
  Task::Pos sum_force{0.0, 0.0};
  const double target_gain = task_config_.target_gain;
  double repellant_gain = task_config_.repellant_gain;
  const double repellant_range = task_config_.repellant_range;
  const double repellant_ellipse_x = task_config_.repellant_ellipse_x;
  const double repellant_ellipse_y = task_config_.repellant_ellipse_y;
  const double repellant_passed_margin_rad = task_config_.repellant_passed_margin_rad;
  // Attractive force towards target
  sum_force += normalize_pos(target_pos - robot_pos) * target_gain; // constant attraction
  
  // // within 0.5m, it shouldn't weaken further, in case repellants are nearby
  // if (sum_force.norm() < target_gain / 2.0) { 
  //   sum_force = normalize_pos(sum_force) * target_gain; // cap at target_gain
  // }

  // // cap the max attractive force at 5m
  // if (sum_force.norm() > 5.0*target_gain) {
  //   sum_force = normalize_pos(sum_force) * target_gain * 5.0;
  // }
  auto dist_to_target = (target_pos - robot_pos).norm();
  if (dist_to_target < 2.0) {
    repellant_gain *= dist_to_target / 2.0; // linearly weaken repellant gain when within 2m of target
  }

  // repellant forces
  for (auto repellant: repellants) {
    double angle = getAngle(target_pos, robot_pos, repellant);
    double reverse_angle = getAngle(robot_pos, target_pos, repellant);
    // ignore if behind. if too far, automatically ignored by range, if behind target, ignore as well
    if (abs(angle) < repellant_passed_margin_rad || abs(reverse_angle) < 1.57) {
      continue;
    }
    bool counter_clockwise = angle < 0;
    sum_force += calculateEllipticalField(robot_pos, repellant, counter_clockwise, repellant_ellipse_x, repellant_ellipse_y, repellant_range) * repellant_gain;
  }

  return normalize_pos(sum_force);
}
