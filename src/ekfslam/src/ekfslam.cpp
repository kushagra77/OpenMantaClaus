#include "ekfslam/ekfslam.hpp"
#include "ekfslam/utils/slam_params.hpp"
#include "std_srvs/srv/set_bool.hpp"

#include <limits>

namespace {
constexpr double kAssociationNoMatchError = 10.0;
constexpr double kAssociationNewUnseenFeatureError = 5.0;
constexpr double kAssociationInvalidIdFallback = 0.001;
constexpr int kFeatureFlagId = 0;
constexpr int kFeatureGateLeftId = 1;
constexpr int kFeatureGateRightId = 2;
constexpr int kFeatureFirstFlareId = 3;
constexpr int kFeatureLastFlareId = 5;
constexpr int kFeatureFirstBucketId = 6;
constexpr int kFeatureLastBucketId = 9;
constexpr int kBucketCount = 4;
}  // namespace

EKFSLAM::EKFSLAM() : Node("ekf_slam") {
    const auto params = load_ekfslam_params(*this, feature_count_);

    features_ = params.feature_names;
    const auto & initial_robot_covariance = params.initial_robot_covariance;

    bearing_std_dev_ = params.bearing_std_dev;
    bearing_range_scale_ = params.bearing_range_scale;
    association_tolerance_rad_ = params.association_tolerance_rad;
    new_association_min_rad_ = params.new_association_min_rad;
    snapshot_time_tolerance_s_ = params.snapshot_time_tolerance_s;
    imu_yaw_std_dev_deg_ = params.imu_yaw_std_dev_deg;
    active_ = params.brainless_run;
    use_imu_update_ = params.use_imu_update;
    // Flare association and initialization tuning.
    flare_association_factor_ = params.flare_association_factor;
    new_flare_dist_ = params.new_flare_dist;
    new_flare_cov_depth_ = params.new_flare_cov_depth;
    new_flare_cov_perp_ = params.new_flare_cov_perp;

    // State layout: [robot_x, robot_y, robot_yaw, landmark_0_x, landmark_0_y, ...].
    state_mu_.setZero();
    double initial_yaw_rad = params.initial_pose_yaw_deg * M_PI / 180.0;
    state_mu_.block<3, 1>(0, 0) << params.initial_pose_x, params.initial_pose_y, initial_yaw_rad;
    state_cov_.setIdentity();
    feature_seen_.assign(features_.size(), false);
    state_cov_.block<3, 3>(0, 0).diagonal() << initial_robot_covariance[0], initial_robot_covariance[1], initial_robot_covariance[2];


    // QoS: Keep only the latest message (Depth 5)
    auto qos_latest = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort();

    // Initialize landmark priors from known course geometry so SLAM starts anchored.
    // Use configured X and Y position priors from params
    init_feature(0, params.flag_x, params.flag_y, ellipse_covariance(0.2, 1.0));  // flag
    init_feature(1, params.gate_x, params.gate_left_y, ellipse_covariance(0.05, 2.0));  // gate_left
    init_feature(2, params.gate_x, params.gate_right_y, ellipse_covariance(0.05, 2.0));  // gate_right
    // Flares: shared X configured by params.flare_x, Y configured individually
    init_feature(3, params.flare_x, params.flare1_y, ellipse_covariance(0.2, 1.0));  // flare_1
    init_feature(4, params.flare_x, params.flare2_y, ellipse_covariance(0.2, 1.0));  // flare_2
    init_feature(5, params.flare_x, params.flare3_y, ellipse_covariance(0.2, 1.0));  // flare_3
    init_feature(6, params.bucket_x, params.bucket1_y, ellipse_covariance(0.2, 2.0));  // bucket_1
    init_feature(7, params.bucket_x, params.bucket2_y, ellipse_covariance(0.2, 2.0));  // bucket_2
    init_feature(8, params.bucket_x, params.bucket3_y, ellipse_covariance(0.2, 2.0));  // bucket_3
    init_feature(9, params.bucket_x, params.bucket4_y, ellipse_covariance(0.2, 2.0));  // bucket_4
    init_feature(10, params.aruco_marker_x, params.aruco_marker_y, ellipse_covariance(0.01, 0.01));  // aruco_marker
    init_feature(11, params.qual_gate_x, params.qual_gate_left_y, ellipse_covariance(0.05, 0.1));  // qual_gate_left
    init_feature(12, params.qual_gate_x, params.qual_gate_right_y, ellipse_covariance(0.05, 0.1));  // qual_gate_right

    // Enforce known structural constraints (gate width, bucket spacing).
    apply_relative_constraint(1, 2, 0.0, -params.gate_width_m); // Gate width
    apply_relative_constraint(11, 12, 0.0, -params.qual_gate_width_m); // Qual gate width
    apply_relative_constraint(6, 7, 0.0, -params.bucket_spacing_m, params.bucket_spacing_y_noise);  // buckets spaced evenly
    apply_relative_constraint(7, 8, 0.0, -params.bucket_spacing_m, params.bucket_spacing_y_noise);
    apply_relative_constraint(8, 9, 0.0, -params.bucket_spacing_m, params.bucket_spacing_y_noise);

    // Initialize the delayed pose covariance block used by frame-triggered updates.
    state_cov_.block<3, 3>(past_idx_, past_idx_).diagonal() << initial_robot_covariance[0], initial_robot_covariance[1], initial_robot_covariance[2];

    // Runtime wiring: odometry source, transform publisher, and subscriptions.
    odom_ = std::make_unique<Odometry>(params.odometry_config);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    // Activation service: allow other nodes (brain) to enable EKF odometry processing.
    activate_srv_ = this->create_service<std_srvs::srv::SetBool>(
        "/ekfslam/activate",
        [this](const std::shared_ptr<std_srvs::srv::SetBool::Request> request,
               std::shared_ptr<std_srvs::srv::SetBool::Response> response) {
            this->active_ = request->data;
            response->success = true;
            response->message = this->active_ ? "ekfslam activated" : "ekfslam deactivated";
            RCLCPP_INFO(this->get_logger(), "%s", response->message.c_str());
        });

    // 1. Initialize the two mutually exclusive callback groups
    state_cbg_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    vision_cbg_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // 2. Create SubscriptionOptions to assign subs to these groups
    auto state_sub_opt = rclcpp::SubscriptionOptions();
    state_sub_opt.callback_group = state_cbg_;

    auto vision_sub_opt = rclcpp::SubscriptionOptions();
    vision_sub_opt.callback_group = vision_cbg_;
    // 3. Assign RC and IMU to the state group.
    rc_sub_ = this->create_subscription<mavros_msgs::msg::RCOut>(
        "/mavros/rc/out", qos_latest, 
        [this](const mavros_msgs::msg::RCOut::SharedPtr msg){
            this->rc_l_ = msg->channels[1];
            this->rc_r_ = msg->channels[0];
            this->rc_back_ = msg->channels[4];
        }, 
        state_sub_opt);

    imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
        "/mavros/imu/data", qos_latest,
        std::bind(&EKFSLAM::imu_sync_callback, this, std::placeholders::_1),
        state_sub_opt);

    // 4. Assign Features and Triggers to the Vision Group
    feature_sub_ = this->create_subscription<interfaces::msg::FeatureObservations>(
        "/cv/feature_observations", 10,
        std::bind(&EKFSLAM::batch_update, this, std::placeholders::_1),
        vision_sub_opt);
    
    // Snapshot pose at camera trigger time for delayed visual updates.
    trigger_sub_ = this->create_subscription<std_msgs::msg::Header>(
        "/camera/frame_trigger", 10, 
        std::bind(&EKFSLAM::trigger_callback, this, std::placeholders::_1),
        vision_sub_opt);
        
    feature_pub_ = this->create_publisher<interfaces::msg::FeatureObservations>("/tasks/feature_observations", 10);

    // 5. Assign the predict timer to the state group.
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(params.predict_period_ms),
        std::bind(&EKFSLAM::predict, this),
        state_cbg_);
}

double EKFSLAM::normalize_angle(double a) {
    return std::atan2(std::sin(a), std::cos(a));
}

Eigen::Matrix2d EKFSLAM::ellipse_covariance(double std_x, double std_y, double theta) {
    Eigen::Matrix2d diag;
    diag << std_x * std_x, 0, 0, std_y * std_y;
    Eigen::Rotation2Dd rot(theta);
    return rot.toRotationMatrix() * diag * rot.toRotationMatrix().transpose();
}

double EKFSLAM::get_bearing_var(double range) const {
    return bearing_std_dev_ * bearing_std_dev_ + bearing_range_scale_ * range;
}

// Returns the best angular association error and updates obs.id/confident on accepted matches.
double EKFSLAM::association_check(interfaces::msg::FeatureObservation &obs, const std::vector<int> & candidate_ids, double override_factor, bool allow_unseen) {
    const int id = obs.id;
    if (id < 0 || static_cast<size_t>(id) >= features_.size()) {
        return kAssociationInvalidIdFallback;
    }

    const double robot_x = state_mu_(past_idx_); // use PAST pose for association
    const double robot_y = state_mu_(past_idx_ + 1);
    const double robot_yaw = state_mu_(past_idx_ + 2);
    const double obs_angle = normalize_angle(obs.bearing);

    int closest_id = -1;
    double closest_diff = kAssociationNoMatchError; // effectively infinity for angle diffs in this pipeline
    double second_closest_diff = kAssociationNoMatchError;

    for (const int candidate_id : candidate_ids) {
        if (candidate_id < 0 || static_cast<size_t>(candidate_id) >= features_.size()) {
            continue;
        }
        if (!feature_seen_[candidate_id]) {
            if (!allow_unseen) {
                continue;
            }
        }

        const int idx = 3 + (2 * candidate_id);
        const double feature_x = state_mu_(idx);
        const double feature_y = state_mu_(idx + 1);
        const double feature_angle = normalize_angle(std::atan2(feature_y - robot_y, feature_x - robot_x) - robot_yaw);
        const double angle_diff = std::abs(normalize_angle(obs_angle - feature_angle));

        if (angle_diff < closest_diff) {
            second_closest_diff = closest_diff;
            closest_diff = angle_diff;
            closest_id = candidate_id;
        } else if (angle_diff < second_closest_diff) {
            second_closest_diff = angle_diff;
        }
    }

    if (closest_id == -1) {
        return kAssociationNoMatchError;
    }
    // Only reachable when allow_unseen is true (flare association path).
    if (!feature_seen_[closest_id]) {
        obs.id = closest_id;
        return kAssociationNewUnseenFeatureError;
    }


    auto angle_tolerance = override_factor > 0.0 ? override_factor * association_tolerance_rad_ : association_tolerance_rad_;
    if (closest_diff <= angle_tolerance) {
        // For flare observations, allow direct closest-match acceptance.
        if (obs.id == 3 || second_closest_diff > new_association_min_rad_) {
            obs.id = closest_id;
            obs.confident = true;
            return 0.0;
        } 
        
    }

    return closest_diff;
}

void EKFSLAM::init_feature(int id, double x, double y, const Eigen::Matrix2d &cov) {
    const int idx = 3 + (2 * id);
    state_mu_(idx) = x;
    state_mu_(idx + 1) = y;
    state_cov_.block<2, 2>(idx, idx) = cov;
}

void EKFSLAM::reinitialize_unseen_flare(int flare_id, double observed_bearing) {
    feature_seen_[flare_id] = true; // prevent re-triggering as unseen within this update pass

    const double r_x = state_mu_(past_idx_);
    const double r_y = state_mu_(past_idx_ + 1);
    const double r_yaw = state_mu_(past_idx_ + 2);

    const double default_depth = new_flare_dist_;
    const double global_angle = normalize_angle(r_yaw + observed_bearing);

    const int f_idx = 3 + (2 * flare_id);

    // 1. Force the landmark directly onto the observed ray.
    state_mu_(f_idx) = r_x + default_depth * std::cos(global_angle);
    state_mu_(f_idx + 1) = r_y + default_depth * std::sin(global_angle);

    // 2. Wipe stale cross-covariances from the previous prior hypothesis.
    state_cov_.row(f_idx).setZero();
    state_cov_.col(f_idx).setZero();
    state_cov_.row(f_idx + 1).setZero();
    state_cov_.col(f_idx + 1).setZero();

    // 3. Seed anisotropic covariance aligned with the observation ray.
    const double std_depth = new_flare_cov_depth_;
    const double std_perp = new_flare_cov_perp_;
    state_cov_.block<2, 2>(f_idx, f_idx) = ellipse_covariance(std_depth, std_perp, global_angle);

    // 4. Re-link landmark cross-covariances with past and current robot poses.
    Eigen::Matrix<double, 2, 3> J_r;
    J_r << 1.0, 0.0, -default_depth * std::sin(global_angle),
           0.0, 1.0,  default_depth * std::cos(global_angle);

    state_cov_.block<2, 3>(f_idx, past_idx_) = J_r * state_cov_.block<3, 3>(past_idx_, past_idx_);
    state_cov_.block<3, 2>(past_idx_, f_idx) = state_cov_.block<2, 3>(f_idx, past_idx_).transpose();

    state_cov_.block<2, 3>(f_idx, 0) = J_r * state_cov_.block<3, 3>(past_idx_, 0);
    state_cov_.block<3, 2>(0, f_idx) = state_cov_.block<2, 3>(f_idx, 0).transpose();
}

void EKFSLAM::publish_tf(const std::string &frame, const std::string &child, const Eigen::Vector3d &p, const rclcpp::Time &t) {
    geometry_msgs::msg::TransformStamped ts;
    ts.header.stamp = t;
    ts.header.frame_id = frame;
    ts.child_frame_id = child;
    ts.transform.translation.x = p(0);
    ts.transform.translation.y = p(1);
    tf2::Quaternion q;
    q.setRPY(0, 0, p(2));
    ts.transform.rotation = tf2::toMsg(q);
    tf_broadcaster_->sendTransform(ts);
}

void EKFSLAM::imu_sync_callback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    // Ensure last_time is valid.
    auto curtime = msg->header.stamp.sec + msg->header.stamp.nanosec * 1e-9;
    if (last_imu_time_ == 0.0) {
        last_imu_time_ = curtime;
        return; // Skip the first update to establish a baseline
    }

    // If not active, keep baseline and skip odometry integration
    if (!active_) {
        last_imu_time_ = curtime;
        return;
    }

    // Calculate dt.
    double dt = curtime - last_imu_time_;
    last_imu_time_ = curtime;

    if (dt <= 0.0 || dt > 1.0) {
        RCLCPP_WARN(this->get_logger(), "IMU Time Glitch Detected! dt = %f. Skipping update.", dt);
        return;
    }

    // Extract yaw from IMU quaternion.
    double yaw = tf2::getYaw(msg->orientation);
    double yaw_cov = msg->orientation_covariance[8];

    // Feed synchronized yaw + RC inputs into the motion model.
    odom_->update_physics(rc_l_, rc_r_, rc_back_, dt, yaw, yaw_cov);
}

void EKFSLAM::trigger_callback(const std_msgs::msg::Header::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    // 1. Copy current pose into delayed-pose slots.
    state_mu_.segment<3>(past_idx_) = state_mu_.segment<3>(0);

    // 2. Copy current robot covariance.
    state_cov_.block<3, 3>(past_idx_, past_idx_) = state_cov_.block<3, 3>(0, 0);

    // 3. Initialize cross-covariance between current and delayed robot pose.
    state_cov_.block<3, 3>(0, past_idx_) = state_cov_.block<3, 3>(0, 0);
    state_cov_.block<3, 3>(past_idx_, 0) = state_cov_.block<3, 3>(0, 0);

    // 4. Copy landmark correlations into delayed-pose cross blocks.
    for (size_t i = 0; i < features_.size(); ++i) {
        int f_idx = 3 + 2 * i;
        state_cov_.block<3, 2>(past_idx_, f_idx) = state_cov_.block<3, 2>(0, f_idx);
        state_cov_.block<2, 3>(f_idx, past_idx_) = state_cov_.block<2, 3>(f_idx, 0);
    }
    // Arm the snapshot for the next visual batch.
    snapshot_time_ = msg->stamp.sec + msg->stamp.nanosec * 1e-9;
    snapshot_ready_ = true;
}

void EKFSLAM::predict() {
    // Prediction step: propagate pose and covariance using odometry increment.
    if (!active_) return;
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto result = odom_->get_delta_and_reset();
    
    // Apply odometry increment to robot state.
    double old_theta = state_mu_(2);
    state_mu_(0) += result.delta(0);
    state_mu_(1) += result.delta(1);
    state_mu_(2) += result.delta(2);
    state_mu_(2) = normalize_angle(state_mu_(2));

    // 1. Jacobian terms for robot orientation effect
    double G_theta_x = -result.delta(1); 
    double G_theta_y =  result.delta(0);

    // 2. Update Robot-Robot Covariance (Top-Left 3x3)
    // P_rr_new = G_r * P_rr * G_r' + Q
    Eigen::Matrix3d G_r = Eigen::Matrix3d::Identity();
    G_r(0, 2) = G_theta_x;
    G_r(1, 2) = G_theta_y;
    state_cov_.block<3, 3>(0, 0) = G_r * state_cov_.block<3, 3>(0, 0) * G_r.transpose();
    state_cov_.block<3, 3>(0, 0) += result.covariance; // Add Q

    // propagate covariance to past pose. same logic as robot-landmark correlation
    state_cov_.block<3, 3>(0, past_idx_) = G_r * state_cov_.block<3, 3>(0, past_idx_);
    state_cov_.block<3, 3>(past_idx_, 0) = state_cov_.block<3, 3>(0, past_idx_).transpose();

    // Propagate robot-landmark correlation so map confidence stays consistent.
    // P_rm_new = G_r * P_rm
    // This spreads the robot's orientation uncertainty into its relationship with the landmarks
    // Since G_r is mostly identity, we only need to update the X and Y rows based on the Theta row.
    int num_landmarks = features_.size();
    for (int i = 0; i < num_landmarks; ++i) {
        int idx = 3 + 2 * i; // Landmark index
        // P_xm' = P_xm + G_theta_x * P_tm
        // P_ym' = P_ym + G_theta_y * P_tm
        state_cov_.block<1, 2>(0, idx) += G_theta_x * state_cov_.block<1, 2>(2, idx);
        state_cov_.block<1, 2>(1, idx) += G_theta_y * state_cov_.block<1, 2>(2, idx);
        
        // Symmetrize (Update Bottom-Left 2x3)
        state_cov_.block<2, 1>(idx, 0) = state_cov_.block<1, 2>(0, idx).transpose();
        state_cov_.block<2, 1>(idx, 1) = state_cov_.block<1, 2>(1, idx).transpose();
    }
    state_cov_ = 0.5 * (state_cov_ + state_cov_.transpose()); // Ensure symmetry

    if (state_mu_.hasNaN() || state_cov_.hasNaN()) {
        RCLCPP_ERROR(this->get_logger(), "FATAL MATH ERROR: NaNs detected in EKF! Resetting state to prevent crash.");
        return;
    }
    publish_transforms();
}

void EKFSLAM::batch_update(const interfaces::msg::FeatureObservations::SharedPtr features) {
    // Update step runs on delayed robot pose captured at camera trigger time.
    if (!active_) return;
    std::lock_guard<std::mutex> lock(state_mutex_);
    double msg_time = features->header.stamp.sec + features->header.stamp.nanosec * 1e-9;
    
    // Snapshot/measurement pairing invariant:
    // each feature batch must correspond to the latest trigger_callback snapshot.
    if (!snapshot_ready_ || std::abs(msg_time - snapshot_time_) > snapshot_time_tolerance_s_) {
        RCLCPP_WARN(this->get_logger(), "YOLO measurement timestamp mismatch or no snapshot ready. Dropping update.");
        return;
    }
    RCLCPP_INFO(this->get_logger(), "received feature observation batch of size %d", features->size);
    snapshot_ready_ = false; // Disarm snapshot until next trigger
    
    std::vector<interfaces::msg::FeatureObservation> valid_observations;
    std::vector<bool> accepted_ids(features_.size(), false);
    
    int buckets_seen = 0;
    for (const auto& obs : features->observations) {
        if (obs.id < 0 || static_cast<size_t>(obs.id) >= features_.size()) {
            RCLCPP_INFO(this->get_logger(), "EKF GOT OUT OF BOUNDS FEATURE ID %d", obs.id);
            continue;
        }

        interfaces::msg::FeatureObservation new_obs = obs;
        // Ambiguous observations are disambiguated by map geometry.
        if (!new_obs.confident) {
            if (features_[new_obs.id].substr(0, 5) == "gate_") {
                // if gate is already seen, try match it in the map
                std::vector<int> candidate_ids = {
                    kFeatureFlagId, // for maingate we compare to flag as well
                    new_obs.id
                };
                
                // For first gate observation, accept and inflate covariance.
                if (!feature_seen_[new_obs.id]) {
                    new_obs.confident = true;
                    new_obs.bearing_cov *= 2.0;
                } else {
                    association_check(new_obs, candidate_ids);
                } 
                
                if (new_obs.id == kFeatureFlagId) {
                    new_obs.confident = false;
                }
            } else if (features_[new_obs.id].substr(0, 10) == "qual_gate_") {
                // if gate is already seen, try match it in the map
                std::vector<int> candidate_ids = {
                    new_obs.id
                };

                association_check(new_obs, candidate_ids);
                if (!feature_seen_[new_obs.id]) {
                    new_obs.confident = true;
                    new_obs.bearing_cov *= 2.0;
                }
            } else if (features_[new_obs.id] == "flag") {
                if (feature_seen_[new_obs.id]) {
                    const std::vector<int> candidate_ids = {
                        new_obs.id
                    };
                    association_check(new_obs, candidate_ids);
                } else {
                    new_obs.confident = true;
                    new_obs.bearing_cov *= 2.0;
                }
            } else if (features_[new_obs.id] == "flare_1") {
                // Compare flare candidate against nearby ambiguous classes.
                std::vector<int> candidate_ids = {
                    new_obs.id,
                    new_obs.id + 1,
                    new_obs.id + 2,
                    kFeatureFlagId,
                };
                // Include gate candidates only after gate is initialized.
                if (feature_seen_[kFeatureGateLeftId] && feature_seen_[kFeatureGateRightId]) {
                    candidate_ids.push_back(kFeatureGateLeftId);
                    candidate_ids.push_back(kFeatureGateRightId);
                }
                double association_error = association_check(new_obs, candidate_ids, flare_association_factor_, true);
                if (new_obs.id <= kFeatureGateRightId) {
                    new_obs.confident = false;
                } else if (association_error == kAssociationNewUnseenFeatureError) {
                    RCLCPP_INFO(this->get_logger(), "MADE NEW TING");
                    new_obs.confident = true;
                    reinitialize_unseen_flare(new_obs.id, new_obs.bearing);
                }
            } else if (features_[new_obs.id].substr(0, 6) == "bucket") {
                // Assumes bucket observations are indexed right-to-left (4,3,2,1).
                int bucket_num = features_[new_obs.id][7] - '0'; // Convert char to int

                if (buckets_locked_) {
                    const std::vector<int> candidate_ids = {
                        kFeatureFirstBucketId,
                        kFeatureFirstBucketId + 1,
                        kFeatureFirstBucketId + 2,
                        kFeatureLastBucketId
                    };
                    association_check(new_obs, candidate_ids, 0.5);
                } else {
                    new_obs.confident = true;
                }

                if (!buckets_locked_ && bucket_num + buckets_seen < kBucketCount && new_obs.color != 0) {
                    new_obs.color = 0;
                }

                if (new_obs.confident) buckets_seen++;
            }
        }
        if (buckets_seen == kBucketCount) buckets_locked_ = true;

        if (!new_obs.confident) {
            continue;
        }

        // Keep only one accepted observation per feature id.
        if (accepted_ids[new_obs.id]) {
            continue;
        }

        accepted_ids[new_obs.id] = true;
        valid_observations.push_back(new_obs);
    }

    for (const auto & valid_obs : valid_observations) {
        feature_seen_[valid_obs.id] = true;
    }

    // Publish filtered observations for task execution.
    interfaces::msg::FeatureObservations obs_msg;
    obs_msg.header.stamp = features->header.stamp;
    obs_msg.size = valid_observations.size();
    obs_msg.observations = valid_observations;
    feature_pub_->publish(obs_msg);
    
    int N = static_cast<int>(valid_observations.size());
    const int imu_measurement_count = use_imu_update_ ? 1 : 0;
    int N_total = N + imu_measurement_count;
    if (N_total == 0) return;

    // Z: measured bearings, h_x: predicted bearings from state.
    Eigen::VectorXd Z(N_total), h_x(N_total);
    Eigen::MatrixXd R = Eigen::MatrixXd::Zero(N_total, N_total);
    Eigen::MatrixXd H = Eigen::MatrixXd::Zero(N_total, state_size_);

    // Use delayed robot pose for visual update alignment.
    double rx = state_mu_(past_idx_), ry = state_mu_(past_idx_ + 1), rt = state_mu_(past_idx_ + 2); 
    int i = 0;
    RCLCPP_INFO(this->get_logger(), "Processing %d feature observations", N);

    for (const auto& feature : valid_observations) {
        int f_idx = 3 + (2 * feature.id);
        double dx = state_mu_(f_idx) - rx;
        double dy = state_mu_(f_idx + 1) - ry;
        double q = dx * dx + dy * dy;
        if (q < 1e-9) {
            continue;
        }

        // Bearing-only observation model: z = atan2(dy, dx) - robot_yaw.
        double predicted = normalize_angle(std::atan2(dy, dx) - rt);
        double observed = normalize_angle(feature.bearing);
        double range = std::sqrt(q);
        double bearing_var = feature.bearing_cov > 0.0 ? feature.bearing_cov : get_bearing_var(range);

        Z(i) = observed;
        h_x(i) = predicted;
        R(i, i) = bearing_var;

        // Jacobian of bearing measurement wrt PAST robot and landmark states.
        H(i, past_idx_) = dy / q;
        H(i, past_idx_ + 1) = -dx / q;
        H(i, past_idx_ + 2) = -1.0;
        H(i, f_idx) = -dy / q;
        H(i, f_idx + 1) = dx / q;

        i++;
    }

    if (use_imu_update_) {
        // Append the absolute IMU yaw observation when enabled.
        double imu_yaw = odom_->get_pose()(2);
        Z(i) = imu_yaw;
        h_x(i) = state_mu_(2); // Predicted yaw is the state yaw

        // Fixed IMU yaw noise model: 0.5 degrees converted to variance.
        double imu_std_dev_rad = imu_yaw_std_dev_deg_ * M_PI / 180.0;
        R(i, i) = imu_std_dev_rad * imu_std_dev_rad;

        // Jacobian for direct yaw observation.
        H(i, 2) = 1.0;

        i++; // Increment total active measurements
    }

    if (i == 0) return;

    Z.conservativeResize(i);
    h_x.conservativeResize(i);
    R.conservativeResize(i, i);
    H.conservativeResize(i, state_size_);

    // Normalize innovation to avoid angle wrap discontinuities near +-pi.
    Eigen::VectorXd innovation = Z - h_x;
    for (int k = 0; k < innovation.size(); ++k) {
        innovation(k) = normalize_angle(innovation(k));
    }

    Eigen::MatrixXd S = H * state_cov_ * H.transpose() + R;
    Eigen::LDLT<Eigen::MatrixXd> ldlt(S);
    if (ldlt.info() != Eigen::Success) {
        RCLCPP_WARN(this->get_logger(), "Skipping EKF batch update: innovation covariance is not decomposable");
        return;
    }

    Eigen::MatrixXd PHt = state_cov_ * H.transpose();
    Eigen::MatrixXd K = ldlt.solve(PHt.transpose()).transpose();
    state_mu_ += K * innovation;
    state_mu_(2) = normalize_angle(state_mu_(2));
    state_mu_(past_idx_ + 2) = normalize_angle(state_mu_(past_idx_ + 2));
    state_cov_ = (Eigen::Matrix<double, state_size_, state_size_>::Identity() - K * H) * state_cov_;

    state_cov_ = 0.5 * (state_cov_ + state_cov_.transpose()); // Ensure symmetry 
    
    RCLCPP_INFO(this->get_logger(), "Batch update completed with %d bearing observations", i);
}

void EKFSLAM::apply_relative_constraint(int id_a, int id_b, double dx_known, double dy_known, double y_noise) {
    int idx_a = 3 + (2 * id_a);
    int idx_b = 3 + (2 * id_b);

    // Innovation: Known distance - Current map distance
    Eigen::Vector2d z(dx_known, dy_known);
    Eigen::Vector2d h_x(state_mu_(idx_b) - state_mu_(idx_a), state_mu_(idx_b + 1) - state_mu_(idx_a + 1));
    Eigen::Vector2d innovation = z - h_x;

    // Jacobian H (2 x state_size_): -1 for Feature A, +1 for Feature B
    Eigen::Matrix<double, 2, state_size_> H = Eigen::Matrix<double, 2, state_size_>::Zero();
    H(0, idx_a) = -1.0; H(0, idx_b) = 1.0;
    H(1, idx_a + 1) = -1.0; H(1, idx_b + 1) = 1.0;

    // Measurement noise: Set to near-zero for rigid constraint
    Eigen::Matrix2d R = ellipse_covariance(1e-6, y_noise);

    // Standard EKF correction for pseudo-measurement constraints.
    Eigen::Matrix2d S = H * state_cov_ * H.transpose() + R;
    Eigen::LDLT<Eigen::Matrix2d> ldlt(S);
    if (ldlt.info() != Eigen::Success) {
        return;
    }

    Eigen::Matrix<double, state_size_, 2> PHt = state_cov_ * H.transpose();
    Eigen::Matrix<double, state_size_, 2> K = ldlt.solve(PHt.transpose()).transpose();
    state_mu_ += K * innovation;
    state_cov_ = (Eigen::Matrix<double, state_size_, state_size_>::Identity() - K * H) * state_cov_;
}

void EKFSLAM::publish_transforms() {
    auto now = this->now();

    // 1. Get Poses
    Eigen::Vector3d o_pose = odom_->get_pose();          // Odom -> Base
    Eigen::Vector3d s_pose = state_mu_.segment<3>(0);    // Map -> Base

    // 2. Publish "odom" -> "base_link" (The easy one)
    publish_tf("odom", "base_link", o_pose, now);

    // 3. Calculate "map" -> "odom" using TF2 Utilities
    // We need: T_map_odom = T_map_base * T_odom_base^-1
    
    // A. Create Transform for Odom -> Base
    tf2::Transform t_odom_base;
    t_odom_base.setOrigin(tf2::Vector3(o_pose(0), o_pose(1), 0.0));
    tf2::Quaternion q_odom; 
    q_odom.setRPY(0, 0, o_pose(2));
    t_odom_base.setRotation(q_odom);

    // B. Create Transform for Map -> Base
    tf2::Transform t_map_base;
    t_map_base.setOrigin(tf2::Vector3(s_pose(0), s_pose(1), 0.0));
    tf2::Quaternion q_map; 
    q_map.setRPY(0, 0, s_pose(2));
    t_map_base.setRotation(q_map);

    // C. The Math: Map -> Odom
    tf2::Transform t_map_odom = t_map_base * t_odom_base.inverse();
    geometry_msgs::msg::TransformStamped ts;
    ts.header.stamp = now;
    ts.header.frame_id = "map";
    ts.child_frame_id = "odom";
    ts.transform = tf2::toMsg(t_map_odom);
    tf_broadcaster_->sendTransform(ts);

    // 5. Publish feature transforms.
    for (size_t i = 0; i < features_.size(); ++i) {
        int idx = 3 + (2 * i);
        Eigen::Vector3d f_pose;
        f_pose << state_mu_(idx), state_mu_(idx + 1), 0.0;
        publish_tf("map", features_[i], f_pose, now);
    }

    // Publish delayed pose for visualization.
    Eigen::Vector3d past_pose = state_mu_.segment<3>(past_idx_);
    publish_tf("map", "past_base_link", past_pose, now);
}

void EKFSLAM::apply_active_flare_constraints() {
    // 1. Define Static X-Axis Boundaries (4.0m to 6.0m)
    const double center_x = 5.0;
    const double half_width_x = 1.0;

    // 2. Define Static Y-Axis Boundaries (-8.0m to +8.0m)
    const double center_y = 0.0;
    const double half_width_y = 8.0;

    const double R_soft = 0.01; // Soft spring for inside the zone

    // Loop through Flares (IDs 3,4,5) and the Flag (ID 0)
    std::vector<int> target_ids = {0, 3, 4, 5}; 

    for (int id : target_ids) {
        if (!feature_seen_[id]) continue; 

        int f_idx = 3 + (2 * id);

        // ==========================================
        // PART A: ENFORCE X-AXIS (4.0 to 6.0)
        // ==========================================
        double flare_x = state_mu_(f_idx);
        double inn_x = 0.0;
        double H_val_x = 0.0;
        double R_eff_x = R_soft;

        if (flare_x < (center_x - half_width_x)) {
            // OUT OF BOUNDS LEFT: Hard Wall Snap
            inn_x = (center_x - half_width_x) - flare_x; 
            H_val_x = 1.0; 
            R_eff_x = 1e-8; // Absolute truth
            state_cov_(f_idx, f_idx) += 0.1; // Inject uncertainty to bypass Kalman stubbornly trusting the camera
        } else if (flare_x > (center_x + half_width_x)) {
            // OUT OF BOUNDS RIGHT: Hard Wall Snap
            inn_x = (center_x + half_width_x) - flare_x; 
            H_val_x = 1.0; 
            R_eff_x = 1e-8;
            state_cov_(f_idx, f_idx) += 0.1;
        } else {
            // INSIDE BOUNDS: Smooth rubber band
            double u_x = (flare_x - center_x) / half_width_x;
            
            // DEADBAND: Let it move completely freely in the middle 75%
            if (std::abs(u_x) < 0.75) {
                H_val_x = 0.0; 
            } else {
                u_x = std::max(std::min(u_x, 0.98), -0.98); 
                double u_x2 = u_x * u_x;
                double one_minus_ux2 = 1.0 - u_x2;
                
                double h_x_val = (u_x2 * u_x) / one_minus_ux2;
                inn_x = 0.0 - h_x_val; 
                
                double dh_du = (3.0 * u_x2 - u_x2 * u_x2) / (one_minus_ux2 * one_minus_ux2);
                H_val_x = dh_du * (1.0 / half_width_x);
            }
        }

        if (H_val_x > 0.0) {
            Eigen::Matrix<double, 1, state_size_> H_x = Eigen::Matrix<double, 1, state_size_>::Zero();
            H_x(0, f_idx) = H_val_x;

            double S_x = (H_x * state_cov_ * H_x.transpose())(0,0) + R_eff_x;
            if (std::abs(S_x) > 1e-9) {
                Eigen::Matrix<double, state_size_, 1> K_x = (state_cov_ * H_x.transpose()) / S_x;
                state_mu_ += K_x * inn_x;
                state_cov_ = (Eigen::Matrix<double, state_size_, state_size_>::Identity() - K_x * H_x) * state_cov_;
            }
        }

        // ==========================================
        // PART B: ENFORCE Y-AXIS (-8.0 to 8.0)
        // ==========================================
        double flare_y = state_mu_(f_idx + 1); 
        double inn_y = 0.0;
        double H_val_y = 0.0;
        double R_eff_y = R_soft;

        if (flare_y < -half_width_y) {
            inn_y = -half_width_y - flare_y;
            H_val_y = 1.0;
            R_eff_y = 1e-8;
            state_cov_(f_idx + 1, f_idx + 1) += 0.1;
        } else if (flare_y > half_width_y) {
            inn_y = half_width_y - flare_y;
            H_val_y = 1.0;
            R_eff_y = 1e-8;
            state_cov_(f_idx + 1, f_idx + 1) += 0.1;
        } else {
            double u_y = flare_y / half_width_y;
            
            if (std::abs(u_y) < 0.75) {
                H_val_y = 0.0;
            } else {
                u_y = std::max(std::min(u_y, 0.98), -0.98); 
                double u_y2 = u_y * u_y;
                double one_minus_uy2 = 1.0 - u_y2;
                
                double h_y_val = (u_y2 * u_y) / one_minus_uy2;
                inn_y = 0.0 - h_y_val;
                
                double dh_du = (3.0 * u_y2 - u_y2 * u_y2) / (one_minus_uy2 * one_minus_uy2);
                H_val_y = dh_du * (1.0 / half_width_y);
            }
        }

        if (H_val_y > 0.0) {
            Eigen::Matrix<double, 1, state_size_> H_y = Eigen::Matrix<double, 1, state_size_>::Zero();
            H_y(0, f_idx + 1) = H_val_y;

            double S_y = (H_y * state_cov_ * H_y.transpose())(0,0) + R_eff_y;
            if (std::abs(S_y) > 1e-9) {
                Eigen::Matrix<double, state_size_, 1> K_y = (state_cov_ * H_y.transpose()) / S_y;
                state_mu_ += K_y * inn_y;
                state_cov_ = (Eigen::Matrix<double, state_size_, state_size_>::Identity() - K_y * H_y) * state_cov_;
            }
        }
    }
    // Symmetrize once at the very end
    state_cov_ = 0.5 * (state_cov_ + state_cov_.transpose());
}


int main(int argc, char * argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<EKFSLAM>();
    rclcpp::executors::MultiThreadedExecutor exec;
    exec.add_node(node);
    exec.spin();
    rclcpp::shutdown();
    return 0;
}