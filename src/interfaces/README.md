# interfaces

`interfaces` defines the custom ROS 2 messages and services shared by runtime packages.

## Purpose

- Provide a single source of truth for cross-package data contracts.
- Keep `brain`, `cv`, `ekfslam`, and `tasks` decoupled from ad-hoc message definitions.

## Messages

### `FeatureObservation.msg`

Single feature detection from CV or EKF SLAM:

- `int32 id` - Feature landmark ID (0-12, see ekfslam landmarks)
- `float64 bearing` - Detected bearing angle in radians (0 = forward, π/2 = left, -π/2 = right)
- `float64 bearing_cov` - Bearing measurement covariance
- `bool confident` - Whether detection confidence is high
- `uint8 color` - Color category (reserved for color-based feature tracking)

### `FeatureObservations.msg`

Batch of feature observations from a single frame/update:

- `std_msgs/Header header` - Timestamp and frame ID
- `int32 size` - Number of observations in batch
- `FeatureObservation[] observations` - Array of individual observations

### `TaskStatus.msg`

Status update from task executor to mission controller:

- Provides state and progress information about current task execution

## Services

### `TaskCommand.srv`

Request for starting or updating a task:

- `string command` - Task name (e.g., "gate", "qual_gate", "flare", "bucket", "aruco", "gripper", "none")
- `bool initial` - Whether this is initial task start (true) or an update (false); affects task initialization behavior

Response:

- `bool success` - Whether command was accepted

### `TaskComplete.srv`

Notification from task executor that a task has completed:

- `bool task_completed` - True if task reached completion condition

Response: (empty)

### `Setup.srv`

Setup contract used for setup-related integration (reserved for future expansion).

Response: (reserved)

## Usage

```bash
colcon build --packages-select interfaces
source install/setup.bash
```

Packages that depend on these definitions should list `interfaces` in their dependencies and source the workspace before running.