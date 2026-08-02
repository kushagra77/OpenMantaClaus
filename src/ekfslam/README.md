# ekfslam

`ekfslam` provides independent odometry publishing (`odometry_node`), robot pose prediction via TF, and landmark map updates using bearing-only feature observations (`ekfslam_node`).

## Purpose

- Estimate vehicle displacement via model-based odometry and publish `odom -> base_link` TF (`odometry_node`).
- Run 30Hz prediction using `odom -> base_link` TF deltas and publish `map -> odom` TF (`ekfslam_node`).
- Apply batched bearing updates from CV observations and publish landmark TF frames.
- Republish validated observations with unique final feature IDs to `/tasks/feature_observations` for task control.

> [!WARNING]
> **Custom Odometry Node Recommendation:**
> Users are strongly urged to create their own odometry node (and modify bringup launch files to point there). The default `odometry_node` is specifically designed for differential-drive thrusters, relies on a fragile `/mavros/rc/out` topic that is highly dependent on specific channel wiring and hardware, and is tuned specifically for the original OpenMantaClaus AUV build. Custom odometry nodes (e.g. VIO or DVL) need only publish `odom -> base_link` on TF at a high frequency (ideally > 50 Hz) for `ekfslam_node` to work seamlessly.

In current runtime, `ekfslam` is the observation bridge between perception and tasks:

- input: `/cv/feature_observations`
- output: `/tasks/feature_observations`

## Main Nodes

- Executable: `odometry_node`
  - Source: `src/ekfslam/src/odometry_node.cpp`
  - Subscribes: `/mavros/rc/out` (`mavros_msgs/msg/RCOut`), `/mavros/imu/data` (`sensor_msgs/msg/Imu`)
  - Publishes: `odom -> base_link` transform on TF
- Executable: `ekfslam_node`
  - Source: `src/ekfslam/src/ekfslam.cpp`
  - Subscribes: `/cv/feature_observations` (`interfaces/msg/FeatureObservations`), `/camera/frame_trigger` (`std_msgs/msg/Header`)
  - TF Lookups: `odom -> base_link`
  - Publishes: `map -> odom` transform, feature landmark TF frames, `/tasks/feature_observations` (`interfaces/msg/FeatureObservations`)


## Landmark Map

The EKF SLAM system maintains 13 predefined landmarks. All initial positions and priors (previously hardcoded) are now fully parameterized under `features.*` in `params.yaml`:

| ID | Feature Name | Default Prior Position (X, Y) | Role |
|----|--|-------------|------|
| 0 | flag | (`flag_x`=5.5, `flag_y`=0.0) | Primary vertical reference landmark |
| 1-2 | gate_left, gate_right | (`gate_x`=16.0, `gate_left_y`=0.75, `gate_right_y`=-0.75) | First gate posts (qualification gate) |
| 3-5 | flare_1, flare_2, flare_3 | (`flare_x`=12.0, `flare1_y`=5.0, `flare2_y`=0.0, `flare3_y`=-5.0) | Three flare targets |
| 6-9 | bucket_1, bucket_2, bucket_3, bucket_4 | (`bucket_x`=23.0, `bucket1_y`=1.5, `bucket2_y`=0.5, `bucket3_y`=-0.5, `bucket4_y`=-1.5) | Four bucket positions with 1.0m spacing |
| 10 | aruco_marker | (`aruco_marker_x`=0.0, `aruco_marker_y`=0.0) | ArUco marker reference |
| 11-12 | qual_gate_left, qual_gate_right | (`qual_gate_x`=10.0, `qual_gate_left_y`=0.75, `qual_gate_right_y`=-0.75) | Alternative gate landmarks for qualification |

**Pool Geometry Constraints:**
- Gate posts separation is parameterized by `constraints.gate_width_m` (default 1.5m)
- Buckets spacing is parameterized by `constraints.bucket_spacing_m` (default 1.0m)
- Initial robot starting pose is parameterized by `initial_pose.x`, `initial_pose.y`, and `initial_pose.yaw_deg` (intended for debug/testing tasks).


## EKF SLAM Process

### Odometry Prediction (25 Hz)

- **Input**: RC PWM commands from `/mavros/rc/out`
- **Odometry model**: 7kg mass, 0.45m thruster width, drag/thrust coefficients are parameterized in the shared launch file
- **Prediction**: Updates robot (x, y, yaw) state through dead reckoning
- **Process noise**: Determined by odometry model dynamics

### Bearing-Only Feature Updates

When observations arrive on `/cv/feature_observations`:

1. **Feature Association**:
   - Matches incoming observations to known landmarks using bearing angle predictions from EKF map
   - Only associates to previously-seen landmarks
   - Closest/second-closest candidate comparison with configurable tolerance

2. **Flare Disambiguation**:
   - Special handling for ambiguous flare observations (gate_left/right also considered as candidates)
   - Only enabled after gate_left has been confirmed
   - Assigns to unseen flare IDs if association error exceeds threshold

3. **EKF Update**:
   - Bearing-only (no range) feature observations
   - Updates both landmark position and robot pose covariance
   - Republishes validated observations on `/tasks/feature_observations`

### Output

- **TF Transforms**: Robot frame, odometry frame, landmark frames (for debug/visualization)
- **Feature Observations**: EKF-validated observations with unique final feature IDs for task control
- **Filtering**: Each landmark appears at most once per batch in output observations

Current odometry covariance note:

- Yaw covariance is sourced from IMU orientation covariance (`orientation_covariance[8]`) when available; `odometry.r_yaw` is only used as fallback process-noise growth when IMU yaw variance is unavailable/non-positive.

## Parameters

Loaded in `src/ekfslam/src/utils/slam_params.cpp`:

- **EKF Configuration**:
  - `initial_pose.x`, `initial_pose.y`, `initial_pose.yaw_deg`: Starting robot pose (YAW in degrees, X/Y in meters). Designed for task testing.
  - `initial_robot_covariance`: [1e-6, 1e-6, 1e-4] - initial (x, y, yaw) covariance
  - `predict_period_ms`: 40 (25 Hz prediction rate)
  - `brainless_run`: false - start EKF SLAM active immediately
  
- **Observation Processing**:
  - `bearing_noise.std_dev`: 0.1 rad - bearing measurement noise
  - `association_tolerance_deg`: 15.0 - angular tolerance for feature association
  - `new_association_min_deg`: Threshold for assigning to unseen landmarks
  
- **Odometry Model**:
  - Mass: 7kg
  - Thruster width: 0.45m
  - Drag and thrust coefficients are parameterized in the shared launch file
  
- **Map Constraints**:
  - Gate separation: 1.5m (fixed geometry)
  - Bucket spacing: 1.0m (fixed geometry)
  
- **Feature Configuration**:
  - `common.feature_names`: List of 13 landmark names
  - `features.*_x`, `features.*_y`: Parameterized X and Y prior positions for course landmarks (flag, gate pillars, qual gate pillars, buckets, flares, and ArUco marker)
  - `snapshot_time_tolerance_s`: frame-trigger tolerance used for batch alignment
  - `imu_yaw_std_dev_deg`: IMU yaw noise used for the optional absolute yaw update

## Usage

```bash
colcon build --packages-select ekfslam interfaces
source install/setup.bash
ros2 run ekfslam ekfslam_node
```