# MantaClaus

ROS 2 workspace for Team MantaClaus (SAUVC 2026).

This repository contains the runtime stack for mission orchestration, perception, SLAM, and task execution for an untethered underwater robot with a 5-thruster configuration.

## Current Scope

- Qualification mission is the primary implemented mission path.
- All task controllers are implemented: `gate`, `qual_gate`, `flare`, `bucket`, `aruco`, and `gripper`.

## Package Overview

- `src/brain`: mission orchestration node.
- `src/manta_bringup`: launch and shared configuration assets.
- `src/cv`: camera capture and feature detection.
- `src/ekfslam`: bearing-only EKF SLAM and TF publication.
- `src/tasks`: task execution and RC override control.
- `src/interfaces`: shared custom ROS messages/services.
- `src/mavros_control`: MAVROS launch bridge and control utility.

Each package has its own `README.md` for details.

## CV Camera Pipeline

- `cv_node` and `testing_node` share the same camera setup and frame preprocessing logic through `src/cv/cv/utils/camera_control.py`.
- Shared camera parameters are declared/loaded in `src/cv/cv/utils/cv_node_params.py`.
- Exposure behavior supports three modes:
- manual override: `manual_override_exposure > 0` forces a fixed manual exposure value.
- camera auto exposure: `enable_auto_exposure = 1` keeps device auto exposure enabled.
- smart manual exposure: `enable_auto_exposure = 0` runs percentile-based exposure tuning with `min_exposure`/`max_exposure` clamping.
- Camera gain is fixed in code (`CAP_PROP_GAIN = 0`) and is intentionally not parameterized.

Current inactive behavior in `cv_node`:

- even when task processing is inactive, one frame is still captured and preprocessed every second to keep camera/exposure state updated.

## Runtime Flow

1. `brain` starts or updates a task by calling `/cv/task_command`.
2. `cv` updates its detector and calls `/tasks/task_command` to start/update `tasks` execution (except `qual_gate` with `initial=false`, which only updates CV-side detector mode).
3. `cv` publishes raw detections on `/cv/feature_observations`.
4. `ekfslam` consumes `/cv/feature_observations`, performs filtering/association, and publishes uniquely validated observations on `/tasks/feature_observations`.
5. `tasks` consumes `/tasks/feature_observations` (not `/cv/feature_observations`), publishes `/mavros/rc/override`, and reports `/tasks/task_status` and `/tasks/task_complete`.

Current EKF association detail:

- uncertain flare association logic is only executed after `gate_left` has been seen.

## SAUVC Qualification Alignment

The qualification maneuver is implemented by `qual_gate` task logic that performs:

1. Forward gate crossing.
2. U-turn.
3. Reverse gate crossing.

This matches SAUVC qualification requirements for two complete gate passes with a U-turn between them.

In orchestration, `brain` still issues `qual_gate` followed by `-qual_gate` in the qualification sequence. The second command is kept to switch `cv` into reverse-direction qual-gate perception behavior even though `tasks::qual_gate` already handles the return pass internally.

## Build and Run

From repository root:

```bash
colcon build
source install/setup.bash
```

Run robot bringup (starts cv, bottom_cv, tasks, ekfslam, and MAVROS):

```bash
ros2 launch manta_bringup robot_bringup.launch.py
```

Run main mission brain node:

```bash
ros2 launch manta_bringup main.launch.py
```

Run qualification brain node:

```bash
ros2 launch manta_bringup qual.launch.py
```

## Useful Commands

```bash
colcon test
colcon test-result --verbose
```

Record a ROS bag with automatic stop (10-minute timeout):

```bash
./scripts/record_bag.sh <bag_name>
```

The script sends `SIGINT` to `ros2 bag record` after the timeout (equivalent to `Ctrl+C`) and records `cv/feature_observations` in addition to core vehicle topics.

## Documentation

- Architecture: `docs/architecture.md`
- Package docs: `src/*/README.md`
