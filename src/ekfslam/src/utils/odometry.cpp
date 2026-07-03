#include "ekfslam/odometry.hpp"

namespace {
constexpr int kNeutralPwm = 1500;
constexpr int kPwmDeadband = 40;
}

Odometry::Odometry(Config cfg) : cfg_(cfg) {
  pose_.setZero();
  last_read_pose_.setZero();
  vel_lin_ = 0.0;
  p_yaw_ = 0.0;
  p_x_ = 0.0;
  p_y_ = 0.0;
  prev_l_rc_ = 1500;
  prev_r_rc_ = 1500;
}

void Odometry::update_physics(int l_rc, int r_rc, int back_rc, double dt, double imu_yaw, double yaw_cov) {
  if (!imu_offset_set_) {
    if (r_rc == kNeutralPwm && back_rc == kNeutralPwm && l_rc == kNeutralPwm) {
      return;
    }
    imu_offset_yaw_ = normalize_angle(imu_yaw);
    imu_offset_set_ = true;
  }
  imu_yaw = normalize_angle(imu_yaw - imu_offset_yaw_);

  l_rc = prev_l_rc_ + cfg_.rc_lag * dt * (l_rc - prev_l_rc_);
  r_rc = prev_r_rc_ + cfg_.rc_lag * dt * (r_rc - prev_r_rc_);

  const double f_l = -force_from_pwm(l_rc); // esc wires are flipped
  const double f_r = force_from_pwm(r_rc);
  const double acc_lin = ((f_l + f_r) - std::max(cfg_.drag_lin * vel_lin_, 1.0)) / cfg_.mass;

  p_yaw_ += cfg_.r_yaw * dt;
  pose_(2) = imu_yaw;

  const double ds_abs = std::abs(vel_lin_ * dt);
  vel_lin_ += acc_lin * dt;
  vel_lin_ = std::max(vel_lin_, 0.0); // Reverse thrust is not modeled.

  pose_(0) += vel_lin_ * dt * std::cos(pose_(2));
  pose_(1) += vel_lin_ * dt * std::sin(pose_(2));

  const double total_dist_noise = ds_abs * cfg_.xy_dist_noise_scaler;
  p_x_ += std::pow(total_dist_noise * std::cos(pose_(2)), 2);
  p_y_ += std::pow(total_dist_noise * std::sin(pose_(2)), 2);

  prev_l_rc_ = l_rc;
  prev_r_rc_ = r_rc;
}

Odometry::OdomResult Odometry::get_delta_and_reset() {
  OdomResult res;
  res.delta = pose_ - last_read_pose_;
  res.delta(2) = normalize_angle(res.delta(2));

  res.covariance.setZero();
  res.covariance(0, 0) = p_x_;
  res.covariance(1, 1) = p_y_;
  res.covariance(2, 2) = p_yaw_;

  last_read_pose_ = pose_;
  p_x_ = 0.0;
  p_y_ = 0.0;
  p_yaw_ = 0.0;
  return res;
}

Eigen::Vector3d Odometry::get_pose() const {
  return pose_;
}

double Odometry::force_from_pwm(double pwm) const {
  if (abs(pwm - kNeutralPwm) < kPwmDeadband) {
    return 0.0;
  }
  return pwm < kNeutralPwm ? (pwm - kNeutralPwm) * cfg_.thrust_k_r : (pwm - kNeutralPwm) * cfg_.thrust_k_f;
}

double Odometry::normalize_angle(double a) {
  return std::atan2(std::sin(a), std::cos(a));
}
