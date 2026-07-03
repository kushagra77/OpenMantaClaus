#ifndef EKFSLAM__UTILS__SLAM_PARAMS_HPP_
#define EKFSLAM__UTILS__SLAM_PARAMS_HPP_

#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "ekfslam/odometry.hpp"

struct EKFSLAMParams {
  std::vector<std::string> feature_names;
  std::vector<double> initial_robot_covariance;
  int predict_period_ms = 100;
  bool brainless_run = false;
  bool use_imu_update = true;
  double bearing_std_dev = 0.1;
  double bearing_range_scale = 0.0005;
  double association_tolerance_rad = 0.05;
  double new_association_min_rad = 20.0 * M_PI / 180.0;
  double snapshot_time_tolerance_s = 0.001;
  double imu_yaw_std_dev_deg = 0.5;
  double gate_width_m = 1.5;
  double qual_gate_width_m = 1.5;
  double bucket_spacing_m = 1.0;
  double bucket_spacing_y_noise = 0.2;
  // Feature X-position priors (course geometry)
  double gate_x = 16.0;
  double qual_gate_x = 7.0;
  double bucket_x = 24.0;
  // Shared flare X-position prior
  double flare_x = 12.0;
  // Flare Y-position priors
  double flare1_y = 5.0;
  double flare2_y = 5.0;
  double flare3_y = -5.0;
  double flag_x = 6.0;
  double aruco_marker_x = -0.3;
  Odometry::Config odometry_config;
  // New flare tuning parameters
  double flare_association_factor = 0.6; // used as override factor when associating flares
  double new_flare_dist = 5.0; // default distance to place a newly-seen flare along bearing
  double new_flare_cov_depth = 0.8; // covariance (std) along ray direction for new flare
  double new_flare_cov_perp = 0.2; // covariance (std) perpendicular to ray for new flare
};

EKFSLAMParams load_ekfslam_params(rclcpp::Node & node, int feature_count);

#endif