# ROS Workspace Architecture

This document describes package responsibilities, node interfaces, and runtime flow for the MantaClaus ROS 2 workspace.

## Workspace Overview

Core subsystems:

- Mission orchestration: `brain`
- Launch and shared configuration: `manta_bringup`
- Perception: `cv`
- Vision testing and bag replay: `cv_testing`
- State estimation and mapping: `ekfslam`
- Task execution and control: `tasks`
- Shared interfaces: `interfaces`
- MAVROS bridge and utility controls: `mavros_control`

Shared runtime parameters are loaded from `src/manta_bringup/launch/params.yaml` by the bringup launch files. `main.launch.py` and `qual.launch.py` only start `brain`; `robot_bringup.launch.py` starts the perception, SLAM, and task nodes.

## Primary Runtime Flow

1. `brain` selects `main_sequence` or `qual_sequence` from `mission.main_run` and issues the first task request to `cv` over `/cv/task_command`.
2. `cv` updates its active task context and forwards task start/update to `tasks` over `/tasks/task_command`.
3. `cv` publishes YOLO feature output on `/cv/feature_observations`.
4. When `debug_yolo` is enabled, `cv_node` draws the YOLO boxes on the original frame before publishing the debug image.
5. `ekfslam` consumes `/cv/feature_observations`, performs map-aware filtering/association, and republishes validated observations on `/tasks/feature_observations`.
6. `tasks` subscribes to `/tasks/feature_observations` and executes control from that EKF-filtered stream.
7. `tasks` publishes `/mavros/rc/override`, `/tasks/task_status`, and completion via `/tasks/task_complete`.
8. `brain` advances through the sequence until completion, then commands `none` through `cv` before the vehicle surfaces.

## Package: `brain`

Purpose:
Mission orchestrator that performs setup, selects mission sequence, dispatches tasks, and advances mission state.

Launch and shared parameter ownership has moved to `manta_bringup`.

Mission behavior:

- `main.launch.py` starts `brain` with `mission.main_run=true`, so the configured main sequence is used.
- `qual.launch.py` starts `brain` with `mission.main_run=false`, so the qualification sequence is used.
- In both modes, `brain` pushes the first task to `cv`, and the `cv -> tasks` handoff drives the rest of the pipeline.

### Node

| Node | Source | Description | Interfaces |
| --- | --- | --- | --- |
| `brain_node` | `src/brain/src/brain.cpp` | Main mission state machine. Performs setup, dispatches commands, and handles completion transitions. | Subscribes: `mavros/state`, `/tasks/task_status`. Publishes: `mavros/setpoint_position/global`. Clients: `mavros/cmd/arming`, `mavros/set_mode`, `/cv/task_command`. Server: `/tasks/task_complete`. |

## Package: `manta_bringup`

Purpose:
Owns the runtime launch files and shared configuration YAMLs for the mission stack.

### Launch

| Launch File | Purpose | Notes |
| --- | --- | --- |
| `src/manta_bringup/launch/robot_bringup.launch.py` | Robot bringup launch | Starts `cv`, `bottom_cv`, `tasks`, and `ekfslam`. |
| `src/manta_bringup/launch/only_mavros.launch.py` | MAVROS-only launch | Starts only MAVROS for isolated FCU/link bringup and testing. |
| `src/manta_bringup/launch/main.launch.py` | Main mission brain launch | Starts `brain` with `mission.main_run=True`. |
| `src/manta_bringup/launch/qual.launch.py` | Qualification brain launch | Starts `brain` with `mission.main_run=False`. |

## Package: `cv`

Purpose:
Camera perception pipeline and YOLO-based observation publisher.

### Nodes

| Node | Source | Description | Interfaces |
| --- | --- | --- | --- |
| `cv_node` | `src/cv/cv/cv_node.py` | Captures frames, runs shared camera preprocessing/exposure control, performs YOLO26n TFLite inference, publishes observations, and handles task-command handoff from `brain` to `tasks`. | Publishes: `/camera/img`, `/cv/feature_observations`. Server: `/cv/task_command`. Client: `/tasks/task_command`. |
| `testing_node` | `src/cv/cv/testing_node.py` | Camera image publisher for debug/performance checks using the same shared camera-control pipeline as `cv_node`. | Publishes: `/camera/img`. |
| Offline replay tooling | `scripts/cv_testing/` | Development-only replay path for recorded images. It is not part of the mission launch stack. | Publishes replayed camera input for downstream CV testing. |

Shared CV camera-control module:

- `src/cv/cv/utils/camera_control.py` centralizes camera property configuration, undistortion/remap, and smart/manual/auto exposure behavior.

YOLO inference and bounding-box conversion are implemented in `src/cv/cv/yolo.py`. Task-specific detection filtering is implemented in `src/cv/cv/utils/filter_detections.py`.

### YOLO Detection Pipeline

The `cv_node` runs a TFLite-quantized YOLO model inference at each frame:

1. **Capture**: Frame from camera device (default 60 FPS)
2. **Preprocess**: Undistortion, exposure control, frame normalization
3. **YOLO Inference**: TFLite model on 256×256 letterboxed frame
4. **Confidence filtering**: Only detections above threshold (0.8 default) are passed downstream
5. **Task-specific filtering**: 
   - Gate detections → left/right post bearings
   - Flag detections → flag bearing
   - Flare detections → flare bearings (generic)
   - Bucket detections → bucket bearings (generic)
   - ArUco detection → ArUco marker bearing (if active)
6. **Bearing conversion**: YOLO bounding box centroids converted to bearing angles using camera intrinsics (fx=533.16, cx=318.89)
7. **Publication**: Raw observations published on `/cv/feature_observations` for EKF filtering

**YOLO Detection Classes:**
- `flag` (id=0): Vertical cylindrical landmark
- `gate` (id=1): Gate frame posts
- `flare` (id=2): Flare targets
- `bucket` (id=3): Bucket objects

Current `cv_node` behavior:

- When no task is active (`task=none`), `cv_node` still captures and preprocesses one frame per second to maintain camera/exposure state; YOLO inference and feature publishing remain disabled.
- When a task is active, `cv_node` runs YOLO at full frame rate and publishes feature observations.

### ArUco Detection

When the `aruco` task is active, `cv_node` supplements YOLO with OpenCV ArUco detection:

- ArUco library (OpenCV 4.7+) detects markers in ID set [0, 1, 2, 3, 6, 7]
- Marker bearing is extracted and published as `aruco_marker` feature observation
- ArUco detection runs alongside YOLO without timing overhead (parallel marker detection)

## Package: `cv_testing`

Purpose:
Development-only image replay package that publishes recorded frames to camera topics for downstream CV nodes.

### Nodes

| Node | Source | Description | Interfaces |
| --- | --- | --- | --- |
| `cv_testing_node` | `scripts/cv_testing/cv_testing/cv_testing_node.py` | Replays images from a configured folder and publishes them as raw ROS images. No detector logic runs in this package. | Publishes: `/camera/img/`. |

### Relationship to `cv`

- `cv_testing` provides replayed image input for testing runs.
- `cv` is the production runtime package used by mission launches and owns detector/inference logic.
- `cv_testing` is not part of the main mission launch flow and does not process detections or forward task commands to `tasks`.

## Package: `ekfslam`

Purpose:
Bearing-only EKF SLAM with odometry prediction and feature update.

### Node

| Node | Source | Description | Interfaces |
| --- | --- | --- | --- |
| `ekfslam_node` | `src/ekfslam/src/ekfslam.cpp` | Maintains robot pose and landmark map, publishes TF, and republishes validated, uniquely-associated feature observations for task control. | Subscribes: `/mavros/rc/out`, `/mavros/imu/data`, `/cv/feature_observations`. Publishes: TF and `/tasks/feature_observations`. |

## Package: `tasks`

Purpose:
Task execution node that converts target behavior into RC override commands.

`tasks` is the downstream controller stage of the main mission pipeline: it does not start tasks on its own and only consumes the EKF-filtered observations published by `ekfslam`.

> [!IMPORTANT]
> **TF Transform Availability Assumption:**
> The task controllers in `tasks` execute unchecked `get_transform()` lookups (e.g. looking up target features, landmarks, and repellants relative to `base_link` or other coordinate frames). This design relies on the assumption that the `ekfslam` node is active and actively publishing TF transforms for all course features at all times once running. If `ekfslam` stops publishing or fails to initialize a landmark, a `tf2::TransformException` will be thrown.

### Node

| Node | Source | Description | Interfaces |
| --- | --- | --- | --- |
| `task_runner` | `src/tasks/src/task_runner.cpp` | Runs active task controller, applies yaw PD control, publishes RC overrides and task status/completion. | Subscribes: `/tasks/feature_observations` (EKF-filtered). Publishes: `/mavros/rc/override`, `/tasks/task_status`. Server: `/tasks/task_command`. Client: `/tasks/task_complete`. |

`tasks` is the downstream controller stage in the main mission pipeline. It does not start tasks on its own; it only reacts to the task command handed off from `cv` and the validated observations from `ekfslam`.

### Task Controller Status

All six task executors are fully implemented:

- **gate**: Navigates through gate using artificial potential field (APF). Calculates attractive forces from gate posts and repulsive forces from flares/flag. Returns completion when past exit margin (forward mode) or before entry margin (reverse mode).
- **qual_gate**: Three-state machine implementing qualification maneuver: forward pass through gate, 180-degree U-turn, reverse pass through gate. Uses APF for positioning and normalized yaw for rotation control.
- **flare**: Sequential scanner for three flare targets. Tracks current target flare, uses APF with flag and other flares as repellants. Supports scan/non-scan modes with different distance thresholds. Returns completion when all three visited.
- **bucket**: Targets blue bucket (locked after scan). Uses APF with different repellant sets for drop vs. pickup modes. Locks bucket colors when within proximity threshold. Returns completion when target reached.
- **aruco**: Navigates to ArUco marker with flag as repellant. Returns completion when marker distance threshold reached (0.5m default).
- **gripper**: Handles servo rotation for object pickup/drop. Uses bearing angle from observations to determine servo angle. Task completion determined by servo angle reaching configurable deadbands (drop/pickup modes) or timeout expiration.

### Task Configuration System

All tasks now use task-specific configuration structs loaded from ROS parameters:

- `TaskConfig`: Shared APF (Artificial Potential Field) configuration for all tasks
  - `target_gain`, `repellant_gain`, `repellant_range`
  - `repellant_ellipse_x`, `repellant_ellipse_y`, `repellant_passed_margin_rad`
- `GateConfig`: Gate task parameters
- `QualGateConfig`: Qualification gate parameters
- `FlareConfig`: Flare search parameters
  - `scan_threshold`, `target_reached_threshold`
- `ArucoConfig`: ArUco marker detection parameters
  - `target_reached_threshold`
- `BucketsConfig`: Bucket task parameters
  - `target_proximity_threshold`, `lock_proximity_threshold`
- `GripperConfig`: Gripper task parameters
  - `task_timeout_s`: Task timeout in seconds
  - `drop_angle_deadband_deg`: Servo angle deadband for drop mode
  - `pickup_angle_deadband_deg`: Servo angle deadband for pickup mode

All configuration values are declared and loaded through `TaskRunnerParams` from `src/manta_bringup/launch/params.yaml`.

## Package: `interfaces`

Purpose:
Custom ROS 2 messages and services used by runtime packages.

### Messages

- `FeatureObservation.msg`
- `FeatureObservations.msg`
- `TaskStatus.msg`

### Services

- `TaskCommand.srv`
- `TaskComplete.srv`
- `Setup.srv`

## Package: `mavros_control`

Purpose:
MAVROS launch/config bridge plus a Python controller utility used for demo/testing flows.

`mavros_control/launch/mavros.launch` is included by the `manta_bringup` launch files.

## SAUVC Qualification Consistency

Current qualifier flow is aligned with SAUVC qualification rules:

- `mission.qual_sequence` issues `qual_gate` followed by `-qual_gate`.
- `qual_gate` handles forward crossing, U-turn, then reverse crossing before completion.
- `-qual_gate` remains necessary to tell `cv` to switch its qual-gate perception mode for the return segment.

Remaining SAUVC scoring/penalty rules (for touch penalties, auto-abort conditions, and final task scoring) are not centrally enforced by this runtime code and are expected to be judged externally during competition.

## Operational Scripts

- `scripts/record_bag.sh` records mission topics to `bags/<name>` and auto-stops recording by sending `SIGINT` after the configured timeout (currently 10 minutes).
- The recorded topic list includes `/cv/feature_observations` for perception-to-SLAM traceability during replay/debug.