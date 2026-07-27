# tasks

`tasks` executes mission-level behaviors and converts them into RC override commands.

It is the downstream controller stage in the main mission pipeline: `brain` drives `cv`, `cv` forwards task commands to `tasks`, and `ekfslam` supplies the filtered feature observations that `tasks` consumes.

## Purpose

- Receive active task command relayed by `cv`.
- Execute task controller logic each control tick.
- Publish `TaskStatus` and signal completion back to `brain`.
- Publish RC overrides to MAVROS.

## Main Node

- Executable: `task_runner`
- Source: `src/tasks/src/task_runner.cpp`

## Implemented Tasks

All six task controllers are fully implemented:

| Task | Source File | Status |
|------|-------------|--------|
| `gate` | `src/tasks/src/controllers/gate.cpp` | Fully implemented |
| `qual_gate` | `src/tasks/src/controllers/qual_gate.cpp` | Fully implemented |
| `flare` | `src/tasks/src/controllers/flare.cpp` | Fully implemented |
| `bucket` | `src/tasks/src/controllers/buckets.cpp` | Fully implemented |
| `aruco` | `src/tasks/src/controllers/aruco.cpp` | Fully implemented |
| `gripper` | `src/tasks/src/controllers/gripper.cpp` | Fully implemented |

## Qual Gate Behavior

`qual_gate` contains the full qualifier maneuver in one controller instance:

1. Forward pass through gate
2. U-turn
3. Reverse pass through gate

This matches SAUVC qualification rules requiring a U-turn between two complete gate crossings.

Current implementation notes:

- `qual_gate` uses `qual_gate_left` and `qual_gate_right` TF landmarks (not `gate_left`/`gate_right`).
- Task TF lookup defaults to `map` as the source frame in `Task::get_transform(...)`.

## Flare Task Behavior

`flare` task sequentially targets and visits three flare objects:

- Tracks current flare target based on color (flare_1, flare_2, flare_3)
- Uses APF with flag and other flares as repellants
- Supports two distance thresholds:
  - **Scan mode** (`scan=true`): larger threshold (2.0m default) for initial detection
  - **Non-scan mode** (`scan=false`): smaller threshold (0.1m default) for precise approach
- Task completes when all three flares have been visited

## Bucket Task Behavior

`bucket` task handles interaction with colored buckets:

- Targets blue bucket (locked after scan to prevent switching)
- Uses different repellant sets and behavior for drop vs. pickup modes
- **Drop mode** (`drop=true`): Approach bucket for dropping object
- **Pickup mode** (`drop=false`): Approach bucket for picking up object
- Bucket color lock: automatically locks bucket color when within `lock_bucket_proximity_threshold_m` (5.0m default)
- Task completes when target bucket reached (distance threshold 1.0m default)

## Gripper Task Behavior

`gripper` task executes pickup or drop operations based on servo angle feedback:

- **Drop mode** (`initial=true`): Holds servo at drop angle. Task completes when servo angle reaches `drop_angle_deadband_deg` or times out after `task_timeout_s`.
- **Pickup mode** (`initial=false`): Moves servo to pickup angle after `servo_sleep` delay, then holds. Task completes when servo angle reaches `pickup_angle_deadband_deg` or times out.

The task uses the following control parameters:

- `servo_90_pwm_diff`: PWM difference applied to RC override for servo positioning (integer, PWM units)
- `gripper_linear_speed`: Speed scale for linear (forward/backward) motion during gripper operation (double, 0.0-1.0)
- `gripper_yaw_speed`: Speed scale for yaw (rotation) motion during gripper operation (double, 0.0-1.0)
- `servo_sleep`: Delay in seconds before moving servo to pickup angle (double, seconds)
- `pickup_sleep`: Delay in seconds between servo movement and start of pickup approach (double, seconds)

## Interfaces

- Subscribes:
	- `/tasks/feature_observations` (`interfaces/msg/FeatureObservations`, published by `ekfslam`)
- Service server:
	- `/tasks/task_command` (`interfaces/srv/TaskCommand`)
- Service client:
	- `/tasks/task_complete` (`interfaces/srv/TaskComplete`)
- Publishes:
	- `/tasks/task_status` (`interfaces/msg/TaskStatus`)
	- `/mavros/rc/override` (`mavros_msgs/msg/OverrideRCIn`)

Current runtime data path:

- `tasks` does not subscribe directly to `/cv/feature_observations`.
- `tasks` executes from the EKF-filtered stream on `/tasks/feature_observations`.

> [!IMPORTANT]
> **TF Transform Availability Assumption:**
> The controller classes in `tasks` perform unchecked `get_transform()` lookups (e.g. for target features, landmarks, and repellants relative to `base_link` or other frames). This is based on the architectural assumption that the `ekfslam` node is running and publishing TF frames for all course features at all times. If a landmark is not published or fails to initialize, a `tf2::TransformException` will be thrown.

## Parameters

Loaded from shared launch parameters in `src/manta_bringup/launch/params.yaml`, including:

### **Control Loop Configuration**
- `control_frequency_hz`: 20.0 Hz - main task execution rate
- `default_speed`: 0.3 - default forward/backward motion scale
- `max_turn_speed`: 0.5 - maximum yaw motion scale
- `kp_turn`: 0.5 - proportional gain for yaw PD control
- `kd_turn`: 0.07 - derivative gain for yaw PD control
- `aggressive_turn_angle_deg`: 45.0 - threshold for reducing turn speed

### **Shared APF (Artificial Potential Field) Configuration**
Applied to all tasks through `TaskConfig`:
- `target_gain`: 5.0 - attractive force strength toward targets
- `repellant_gain`: 6.0 - repulsive force strength from obstacles
- `repellant_range`: 6.0m - range at which repulsive forces activate
- `repellant_ellipse_x`, `repellant_ellipse_y`: 1.0, 1.0 - elliptical shape of repulsive field
- `repellant_passed_margin_rad`: 1.0 - angle threshold for ignoring obstacles behind robot

### **Gate Configuration** (`gate.*`)
- `ellipse_x`: 5.0 - APF attractive field shape (forward-back)
- `ellipse_y`: 2.0 - APF attractive field shape (left-right)
- `forward_exit_margin_m`: 1.0 - distance past gate for completion (forward mode)
- `apf_target_x_offset_m`: -1.0 - horizontal offset for APF target
- `gate_gain`: 5.0 - task-specific attractive force gain
- `blind_gate_gain_factor`: 0.7 - multiplier applied when neither gate pillar is visible
- `repellant_gain`: 3.0 - task-specific repulsive force gain
- `repellant_range`: 5.0m - task-specific repulsive field range
- `repellant_ellipse_x`, `repellant_ellipse_y`: 1.5, 1.0 - elliptical shape of task repulsive field
- `repellant_passed_margin_m`: 1.0 - angle threshold for task-specific repulsive field
- `repellant_names`: `["flare_1", "flare_2", "flare_3"]` - list of active features treated as repellants (note: the flag is always a default repellant).

### **Qualification Gate Configuration** (`qual_gate.*`)
- `angle_tolerance_deg`: 1.5 - yaw alignment threshold for turn completion
- `ellipse_x`: 5.0, `ellipse_y`: 2.0 - APF field shape
- `forward_exit_margin_m`: 1.5 - exit distance for forward pass
- `backward_exit_margin_m`: 5.0 - exit distance for backward pass
- `turn_target_yaw_offset_deg`: -179.0 - target yaw offset for 180° U-turn
- `apf_target_x_offset_m`: 1.0 - horizontal offset for APF target

### **Flare Configuration** (`flare.*`)
- `scan_target_reached_threshold_m`: 2.0 - distance threshold in scan mode (initial detection)
- `target_reached_threshold_m`: 0.1 - distance threshold in non-scan mode (precise approach)

### **Bucket Configuration** (`buckets.*`)
- `target_reached_threshold_m`: 1.0 - distance threshold for task completion
- `lock_bucket_proximity_threshold_m`: 5.0 - distance at which bucket color is locked
- `repellant_names`: `["flare_1", "flare_2", "flare_3", "gate_left", "gate_right"]` - list of active features treated as repellants in buckets APF.

### **ArUco Configuration** (`aruco.*`)
- `target_reached_threshold_m`: 5.0 - distance threshold for task completion (marker detection range)

### **Gripper Configuration** (`gripper.*`)
- `task_timeout_s`: 20.0 - timeout duration in seconds
- `drop_angle_deadband_deg`: 35.0 - servo angle deadband for drop mode completion
- `pickup_angle_deadband_deg`: 15.0 - servo angle deadband for pickup mode completion

### **Gripper Control Parameters** (applies during gripper task)
- `servo_90_pwm_diff`: 700 - PWM difference for 90° servo rotation
- `gripper_linear_speed`: 0.15 - forward/backward motion speed scale during gripper task
- `gripper_yaw_speed`: 0.2 - yaw motion speed scale during gripper task
- `servo_sleep`: 2.0 - delay (seconds) before moving servo in pickup mode
- `pickup_sleep`: 5.0 - delay (seconds) after servo movement before starting approach

### **Common Features**
- `common.feature_names`: List of feature names (flag, gate_left, gate_right, flare_1, flare_2, flare_3, bucket_1, bucket_2, bucket_3, bucket_4, aruco_marker, qual_gate_left, qual_gate_right)
- `common.feature_indices`: Index mappings for features in state tracking

## Usage

```bash
colcon build --packages-select tasks interfaces
source install/setup.bash
ros2 run tasks task_runner
```