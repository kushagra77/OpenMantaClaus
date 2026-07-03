# ekfslam

`ekfslam` provides robot pose prediction and landmark map updates using bearing-only feature observations.

## Purpose

- Fuse IMU + RC-derived odometry in EKF prediction.
- Apply batched bearing updates from CV observations.
- Publish TF transforms for robot and landmarks.
- Republish validated observations with unique final feature IDs to `/tasks/feature_observations` for task control.

Current odometry covariance note:

- Yaw covariance is sourced from IMU orientation covariance (`orientation_covariance[8]`) when available; `odometry.r_yaw` is only used as fallback process-noise growth when IMU yaw variance is unavailable/non-positive.

In current runtime, `ekfslam` is the observation bridge between perception and tasks:

- input: `/cv/feature_observations`
- output: `/tasks/feature_observations`

Association behavior in current code:

- Uncertain observations are associated against candidate landmark IDs using map-predicted bearing angles.
- Association only considers candidate IDs that have already been seen in a previously accepted update.
- The closest and second-closest seen candidates are compared; an association is accepted only when the closest candidate is within `association_tolerance_deg` and the second-closest candidate is outside `2 * association_tolerance_deg`.
- Flare disambiguation path is currently gated until `gate_left` has been seen.
- In that flare path, candidates include flare IDs plus gate IDs (`gate_left`, `gate_right`) for angle-based disambiguation.
- If a flare observation is too far from any seen candidate, the code may assign it to the first unseen flare ID when the closest association error exceeds `new_association_min_deg`.
- Accepted observations mark their final IDs as seen.
- Republished observations are filtered so each final feature ID appears at most once per batch.

## Main Node

- Executable: `ekfslam_node`
- Source: `src/ekfslam/src/ekfslam.cpp`

## Interfaces

- Subscribes:
	- `/mavros/rc/out` (`mavros_msgs/msg/RCOut`)
	- `/mavros/imu/data` (`sensor_msgs/msg/Imu`)
	- `/cv/feature_observations` (`interfaces/msg/FeatureObservations`)
- Publishes:
	- TF transforms (`map`, `odom`, `base_link`, feature frames)
	- `/tasks/feature_observations` (`interfaces/msg/FeatureObservations`)

## Landmark Map

The EKF SLAM system maintains 13 predefined landmarks (pool geometry is fixed and known):

| ID | Feature Name | Initial Position | Role |
|----|--|-------------|------|
| 0 | flag | (6.0, 0.0) | Primary vertical reference landmark |
| 1-2 | gate_left, gate_right | (16.0, ±0.75) | First gate posts (qualification gate) |
| 3-5 | flare_1, flare_2, flare_3 | (12.0, ±5.0) | Three flare targets |
| 6-9 | bucket_1, bucket_2, bucket_3, bucket_4 | (24.0, ±1.5 to ±0.5) | Four bucket positions with 1.0m spacing |
| 10 | aruco_marker | (-0.3, 0.0) | ArUco marker (starts at robot position) |
| 11-12 | qual_gate_left, qual_gate_right | (10.0, ±0.75) | Alternative gate landmarks for qualification |

**Pool Geometry Constraints:**
- Gate posts fixed at 1.5m separation
- Buckets arranged in line with 1.0m spacing
- Flag and ArUco positioned as primary references

## EKF SLAM Process

### Odometry Prediction (25 Hz)

- **Input**: RC PWM commands from `/mavros/rc/out`
- **Odometry model**: 7kg mass, 0.45m thruster width, drag/thrust coefficients
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
  - `initial_robot_covariance`: [1e-6, 1e-6, 1e-4] - initial (x, y, yaw) covariance
  - `predict_period_ms`: 40 (25 Hz prediction rate)
  - `brainless_run`: false - start EKF SLAM active immediately
  
- **Observation Processing**:
  - `bearing_noise.std_dev`: 0.1 rad - bearing measurement noise
  - `association_tolerance_deg`: 5.0 - angular tolerance for feature association
  - `new_association_min_deg`: Threshold for assigning to unseen landmarks
  
- **Odometry Model**:
  - Mass: 7kg
  - Thruster width: 0.45m
  - Drag and thrust coefficients (in code)
  
- **Map Constraints**:
  - Gate separation: 1.5m (fixed geometry)
  - Bucket spacing: 1.0m (fixed geometry)
  
- **Feature Configuration**:
  - `common.feature_names`: List of 13 landmark names
  - `common.feature_indices`: Index mapping for feature access

## Usage

```bash
colcon build --packages-select ekfslam interfaces
source install/setup.bash
ros2 run ekfslam ekfslam_node
```