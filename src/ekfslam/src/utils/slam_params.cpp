#include "ekfslam/utils/slam_params.hpp"

#include <cmath>

namespace {
const std::vector<std::string> kDefaultFeatureNames = {
  "flag",
  "gate_left",
  "gate_right",
  "flare_1",
  "flare_2",
  "flare_3",
  "bucket_1",
  "bucket_2",
  "bucket_3",
  "bucket_4",
  "aruco_marker",
  "qual_gate_left",
  "qual_gate_right",
};

std::vector<double> validated_vector(
  rclcpp::Node & node,
  const std::string & name,
  const std::vector<double> & fallback,
  size_t expected_size)
{
  auto values = node.get_parameter(name).as_double_array();
  if (values.size() != expected_size) {
    RCLCPP_WARN(
      node.get_logger(),
      "Parameter %s expected %zu values, using defaults",
      name.c_str(),
      expected_size);
    return fallback;
  }
  return values;
}

}

EKFSLAMParams load_ekfslam_params(rclcpp::Node & node, int feature_count)
{
  EKFSLAMParams params;

  node.declare_parameter<std::vector<std::string>>("common.feature_names", kDefaultFeatureNames);
  node.declare_parameter<std::vector<double>>("initial_robot_covariance", {1e-6, 1e-6, 1e-4});
  node.declare_parameter("predict_period_ms", params.predict_period_ms);
  node.declare_parameter("brainless_run", params.brainless_run);
  node.declare_parameter("use_imu_update", params.use_imu_update);
  node.declare_parameter("bearing_noise.std_dev", params.bearing_std_dev);
  node.declare_parameter("bearing_noise.range_scale", params.bearing_range_scale);
  node.declare_parameter("association_tolerance_deg", params.association_tolerance_rad * 180.0 / M_PI);
  node.declare_parameter("new_association_min_deg", params.new_association_min_rad * 180.0 / M_PI);
  node.declare_parameter("snapshot_time_tolerance_s", params.snapshot_time_tolerance_s);
  node.declare_parameter("imu_yaw_std_dev_deg", params.imu_yaw_std_dev_deg);
  node.declare_parameter("constraints.gate_width_m", params.gate_width_m);
  node.declare_parameter("constraints.qual_gate_width_m", params.qual_gate_width_m);
  node.declare_parameter("constraints.bucket_spacing_m", params.bucket_spacing_m);
  node.declare_parameter("constraints.bucket_spacing_y_noise", params.bucket_spacing_y_noise);
  // Feature position priors.
  node.declare_parameter("features.gate_x", params.gate_x);
  node.declare_parameter("features.qual_gate_x", params.qual_gate_x);
  node.declare_parameter("features.bucket_x", params.bucket_x);
  node.declare_parameter("features.flare_x", params.flare_x);
  node.declare_parameter("features.flare1_y", params.flare1_y);
  node.declare_parameter("features.flare2_y", params.flare2_y);
  node.declare_parameter("features.flare3_y", params.flare3_y);
  node.declare_parameter("features.flag_x", params.flag_x);
  node.declare_parameter("features.aruco_marker_x", params.aruco_marker_x);
  node.declare_parameter("odometry.mass", params.odometry_config.mass);
  node.declare_parameter("odometry.width", params.odometry_config.width);
  node.declare_parameter("odometry.drag_lin", params.odometry_config.drag_lin);
  node.declare_parameter("odometry.thrust_k_f", params.odometry_config.thrust_k_f);
  node.declare_parameter("odometry.thrust_k_r", params.odometry_config.thrust_k_r);
  node.declare_parameter("odometry.rc_lag", params.odometry_config.rc_lag);
  node.declare_parameter("odometry.xy_dist_noise_scaler", params.odometry_config.xy_dist_noise_scaler);
  node.declare_parameter("odometry.r_yaw", params.odometry_config.r_yaw);
  // Flare tuning parameters.
  node.declare_parameter("flare_association_factor", params.flare_association_factor);
  node.declare_parameter("new_flare_dist", params.new_flare_dist);
  node.declare_parameter("new_flare_cov_depth", params.new_flare_cov_depth);
  node.declare_parameter("new_flare_cov_perp", params.new_flare_cov_perp);

  params.feature_names = node.get_parameter("common.feature_names").as_string_array();
  if (params.feature_names.size() != static_cast<size_t>(feature_count)) {
    RCLCPP_WARN(node.get_logger(), "Parameter common.feature_names expected %d values, using defaults", feature_count);
    params.feature_names = kDefaultFeatureNames;
  }

  params.initial_robot_covariance = validated_vector(node, "initial_robot_covariance", {1e-6, 1e-6, 1e-4}, 3);

  params.predict_period_ms = node.get_parameter("predict_period_ms").as_int();
  params.brainless_run = node.get_parameter("brainless_run").as_bool();
  params.use_imu_update = node.get_parameter("use_imu_update").as_bool();
  params.bearing_std_dev = node.get_parameter("bearing_noise.std_dev").as_double();
  params.bearing_range_scale = node.get_parameter("bearing_noise.range_scale").as_double();
  params.association_tolerance_rad = node.get_parameter("association_tolerance_deg").as_double() * M_PI / 180.0;
  params.new_association_min_rad = node.get_parameter("new_association_min_deg").as_double() * M_PI / 180.0;
  params.snapshot_time_tolerance_s = node.get_parameter("snapshot_time_tolerance_s").as_double();
  params.imu_yaw_std_dev_deg = node.get_parameter("imu_yaw_std_dev_deg").as_double();
  params.gate_width_m = node.get_parameter("constraints.gate_width_m").as_double();
  params.qual_gate_width_m = node.get_parameter("constraints.qual_gate_width_m").as_double();
  params.bucket_spacing_m = node.get_parameter("constraints.bucket_spacing_m").as_double();
  params.bucket_spacing_y_noise = node.get_parameter("constraints.bucket_spacing_y_noise").as_double();
  // Read feature position priors.
  params.gate_x = node.get_parameter("features.gate_x").as_double();
  params.qual_gate_x = node.get_parameter("features.qual_gate_x").as_double();
  params.bucket_x = node.get_parameter("features.bucket_x").as_double();
  params.flare_x = node.get_parameter("features.flare_x").as_double();
  params.flare1_y = node.get_parameter("features.flare1_y").as_double();
  params.flare2_y = node.get_parameter("features.flare2_y").as_double();
  params.flare3_y = node.get_parameter("features.flare3_y").as_double();
  params.flag_x = node.get_parameter("features.flag_x").as_double();
  params.aruco_marker_x = node.get_parameter("features.aruco_marker_x").as_double();
  params.odometry_config.mass = node.get_parameter("odometry.mass").as_double();
  params.odometry_config.width = node.get_parameter("odometry.width").as_double();
  params.odometry_config.drag_lin = node.get_parameter("odometry.drag_lin").as_double();
  params.odometry_config.thrust_k_f = node.get_parameter("odometry.thrust_k_f").as_double();
  params.odometry_config.thrust_k_r = node.get_parameter("odometry.thrust_k_r").as_double();
  params.odometry_config.rc_lag = node.get_parameter("odometry.rc_lag").as_double();
  params.odometry_config.xy_dist_noise_scaler = node.get_parameter("odometry.xy_dist_noise_scaler").as_double();
  params.odometry_config.r_yaw = node.get_parameter("odometry.r_yaw").as_double();
  // Read flare tuning parameters.
  params.flare_association_factor = node.get_parameter("flare_association_factor").as_double();
  params.new_flare_dist = node.get_parameter("new_flare_dist").as_double();
  params.new_flare_cov_depth = node.get_parameter("new_flare_cov_depth").as_double();
  params.new_flare_cov_perp = node.get_parameter("new_flare_cov_perp").as_double();

  return params;
}
