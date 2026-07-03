# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Project Overview

ROS2 workspace for an underwater robot for the SAUVC competition. [rulebook](https://sauvc.org/rulebook/)

The robot has a 5-thruster configuration with two forward facing thrusters and three vertical upwards thrusters. It is controlled like a differential drive robot at 2D slice of a pool. Vision is handled by a monocular camera with a combination of traditional CV and YOLO. Feature-based EKF SLAM is used for mapping with bearing only updates from CV. Artificial vector fields are used for path planning.

## Build, Test, and Development Commands
Use standard ROS 2 workspace commands from the repository root:
- `colcon build`: build all packages for local development.
- `colcon test`: run package tests and ament linters across the workspace.
- `colcon test-result --verbose`: inspect failing tests after a run.
- `source install/setup.bash`: load the built workspace before `ros2 run` or `ros2 launch`.
- `ros2 launch brain robot_bringup.launch.py`: launch `cv`, `tasks`, `ekfslam`, and MAVROS.
- `ros2 launch brain main.launch.py`: launch the brain node for the main competition run.
- `ros2 launch brain qual.launch.py`: launch the brain node for the qualification run.

## Workspace Structure

```
src/
├── brain/          # Behaviour control (ament_cmake)
├── cv/             # Computer vision (ament_python)
├── ekfslam/        # SLAM package with odometry and mapping (ament_cmake)
├── tasks/          # Path planning and control for every task (ament_cmake)
├── interfaces/     # custom ros interfaces
└── mavros_control/ # MAVROS launch bridge and controller utility
```

Additional development-only package:

```text
scripts/cv_testing/  # CV pipeline development and rosbag replay package (ament_python)
```

## Architecture

The runtime architecture is documented in detail in `docs/architecture.md` and is organized into these subsystems:

- **Mission orchestration**: `brain` handles startup sequencing, task dispatch, and mission progression.
- **Perception**: `cv` is the production perception package used by mission launches. It publishes feature observations and forwards task commands.
- **Vision testing**: `cv_testing` is a development package used to iterate on the CV pipeline, replay ROS bags, and validate detectors before redeploying the same logic back into `cv`.
- **State estimation and mapping**: `ekfslam` runs odometry prediction and batched feature updates.
- **Task execution and control**: `tasks` runs task executors and publishes RC override commands.
- **Shared interfaces**: `interfaces` defines common messages/services used by runtime packages.

Primary runtime flows:
- `brain` → `/cv/task_command` → `cv` → `/tasks/task_command` → `tasks`
- `cv` → `/cv/feature_observations` → `ekfslam` → `/tasks/feature_observations` → `tasks`
- `tasks` → `/tasks/task_status` and `/tasks/task_complete` → `brain`
- `mavros` state/IMU/RC topics feed `brain` and `ekfslam`, while `tasks` publishes `/mavros/rc/override`

Development/testing flow:
- `cv_testing` subscribes to `/cv/image` or replays images from a folder, publishes annotated images plus feature observations, and is used to validate detector changes before copying them into `cv`.

## Package Types

- **ament_python** packages: `setup.py` + `setup.cfg` + `package.xml`
- **ament_cmake** packages: `CMakeLists.txt` + `package.xml`
## Package Layout Convention for ament_cmake AND ament_python

Use the following structure for all runtime packages in this repository:

- Keep only executable entrypoint files directly under the package root module directory (for example `cv_node.py`, `testing_node.py` under `src/cv/cv/`).
- Place reusable implementation code (detectors, controllers, helpers, algorithms) in dedicated subdirectories such as `detectors/`, `controllers/`, or `utils/`.
- Avoid placing non-executable library modules directly next to entrypoint files in the package root.
- Preserve ROS executable names and behavior when refactoring layout (update imports only).

## Architecture Documentation

Read `docs/architecture.md` when you need package-level context about the repository. Keep it aligned with code changes: when modifying nodes, topics, packages, or package responsibilities, make the smallest possible update that reflects the change and preserve the document’s existing headings, tables, and formatting conventions. ALWAYS maintain all documentation, including all README files and architecture.md to be coherent with the latest changes.