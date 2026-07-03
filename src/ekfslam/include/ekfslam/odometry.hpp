#ifndef ODOMETRY_HPP
#define ODOMETRY_HPP

#include <Eigen/Dense>
#include <cmath>

class Odometry {
public:
    struct Config {
        double mass = 7.0;
        double width = 0.45;

        // ---------- factors to tune ----------
        double drag_lin = 8.0; // tune a linear drag coefficient
        double thrust_k_f = 4.0/400.0; // forward thrust linear approximation
        double thrust_k_r = 3.5/400.0; // reverse thrust linear approximation
        double rc_lag = 5.0; // first order response of rc commands per second
        
        // -- process noise parameters to tune --
        double xy_dist_noise_scaler = 0.15; // Total distance-based noise coefficient
        double r_yaw = 0.005;              // IMU measurement noise
    };

    struct OdomResult {
        Eigen::Vector3d delta;
        Eigen::Matrix3d covariance;
    };

    explicit Odometry(Config cfg);

    
    /**
     * @brief Synchronous physics update called from IMU callback.
     */
    void update_physics(int l_rc, int r_rc, int back_rc, double dt, double imu_yaw, double yaw_cov = -1.0);
    
    /**
     * @brief Harvests the displacement and uncertainty for the SLAM step.
     */
    OdomResult get_delta_and_reset();
    
    Eigen::Vector3d get_pose() const;

private:
    Config cfg_;
    Eigen::Vector3d pose_;
    Eigen::Vector3d last_read_pose_;
    double vel_lin_;
    
    double p_yaw_;
    double imu_offset_yaw_;
    bool imu_offset_set_ = false;
    double p_x_;
    double p_y_;
    int prev_l_rc_;
    int prev_r_rc_;
    
    // helper funcs
    double force_from_pwm(double pwm) const;
    static double normalize_angle(double a);
};
#endif