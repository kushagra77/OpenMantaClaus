#ifndef TASK_CONFIG_HPP
#define TASK_CONFIG_HPP

#include <cmath>

struct TaskConfig {
  double target_gain = 5.0;
  double repellant_gain = 3.0;
  double repellant_range = 2.0;
  double repellant_ellipse_x = 1.5;
  double repellant_ellipse_y = 1.0;
  double repellant_passed_margin_rad = 0.5;
};

#endif
