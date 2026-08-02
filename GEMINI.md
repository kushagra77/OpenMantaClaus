# GEMINI — Repository Guide

This file describes the current repository layout and where to find high-level project information. Use `docs/architecture.md` and each package README for detailed developer guidance.

## Short Project Overview

- ROS 2 workspace implementing mission orchestration, monocular perception (YOLO + ArUco), bearing-only EKF SLAM, and task execution for a 5-thruster underwater vehicle.
- The workspace mixes `ament_cmake` C++ nodes and `ament_python` Python nodes.

## High-level structure

Top-level layout (important folders only):

```
src/                     # all ROS packages
	brain/                 # mission orchestration (C++)
	manta_bringup/         # runtime launches and shared params
	cv/                    # perception and YOLO pipeline (Python)
	ekfslam/               # EKF SLAM and observation filtering (C++)
	tasks/                 # task executors and RC override (C++)
	interfaces/            # custom ROS messages and services
	mavros_control/        # MAVROS include and helper utilities
scripts/                 # dataset tooling, training helpers, scripts
docs/                    # architecture and higher-level docs
```

Note: `manta_bringup` is now the owner of the launch files and shared YAMLs; `brain` is node-only.

## Runtime launch split

- `ros2 launch manta_bringup robot_bringup.launch.py` — starts `cv`, `bottom_cv`, `tasks`, `odometry`, and `ekfslam` (vehicle bringup).
- `ros2 launch manta_bringup main.launch.py` — starts `brain` for the main mission sequence.
- `ros2 launch manta_bringup qual.launch.py` — starts `brain` for the qualification mission.

This split means you typically run `robot_bringup` first (bring up perception, odometry, SLAM, and controllers), then start `brain` with `main.launch.py` or `qual.launch.py`.

## Primary runtime flows (topics and handoff)

- Task orchestration: `brain` → `/cv/task_command` → `cv` → `/tasks/task_command` → `tasks`
- Odometry → TF: `odometry` → `odom -> base_link` TF
- Perception → SLAM: `cv` → `/cv/feature_observations` → `ekfslam` (30Hz predict via `odom -> base_link` TF) → `map -> odom` TF & `/tasks/feature_observations` → `tasks`
- Task lifecycle: `tasks` → `/tasks/task_status` and `/tasks/task_complete` → `brain`

`tasks` publishes RC override messages to MAVROS when active; `mavros` state and IMU topics feed `brain` and `odometry`.

## Key developer paths

- Architecture overview: `docs/architecture.md`
- CV params and defaults: `src/cv/cv/utils/cv_node_params.py`
- EKF params: `src/ekfslam/src/utils/slam_params.cpp`
- Task config and constants: `src/tasks/include/tasks/task_protocol.hpp` and `src/brain/src/utils/brain_params.cpp`
- Launch/config owner: `src/manta_bringup/launch/` (includes `params.yaml`, `brain_qual.yaml`, `dummy_cv.yaml`)

## Dataset and training tooling

- YOLO training data and model runs: `scripts/yolo/runs/detect/`
- Dataset preprocessing helper: `scripts/train_preprocess_script.py`

## Build & run (developer quickstart)

From repository root:

```bash
colcon build
source install/setup.bash
```

Bring up vehicle systems:

```bash
ros2 launch manta_bringup robot_bringup.launch.py
```

Start mission orchestrator (in a separate terminal):

```bash
ros2 launch manta_bringup main.launch.py
# or
ros2 launch manta_bringup qual.launch.py
```

## Notes for contributors

- Keep README files and `docs/architecture.md` consistent with code changes (source of truth: parameter loader code and launch files).
- For CV changes, verify detector behavior with `scripts/cv_testing` before modifying the production `cv` package.

---
Last updated: repository sync (reflects `manta_bringup` ownership of launches and updated package README locations).