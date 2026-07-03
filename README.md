# OpenMantaClaus

OpenMantaClaus is a competitve robotics project for an untethered underwater robot built for SAUVC 2026. The hardware stack has 5 thrusters, deigned specifically to be a cheap, accessible entry into AUVs. The software stack combines mission orchestration, monocular perception, bearing-only EKF SLAM, and task execution for a 5-thruster vehicle. Custom YOLO datasets and models are also

## Purpose

The robot is designed to execute the competition mission pipeline end to end: the `brain` node selects the mission sequence, `cv` detects visual targets, `ekfslam` filters and associates observations, and `tasks` converts those observations into RC override commands.

## Mechanical Design

- 5-thruster configuration.
- Two forward-facing thrusters for planar motion.
- Three upward-facing thrusters for vertical control.
- Control is treated like a differential-drive-style planar system for mission logic.
- A monocular camera provides the primary visual feed.

## YOLO Dataset

- The CV stack uses a YOLO26n detector for mission perception.
- Detection targets include `flag`, `gate`, `flare`, `bucket`, and ArUco markers.
- Dataset generation and preprocessing utilities live in `scripts/train_preprocess_script.py` and the `scripts/yolo/` workspace.
- Training runs and exported model artifacts are stored under `scripts/yolo/runs/detect/`.

## Bill of Materials

- Propulsion: 5 thrusters, ESCs, and the associated mounting hardware.
- Perception: monocular camera and supporting compute.
- Navigation and control: flight controller, MAVROS bridge, and onboard companion computer.
- Structure: frame, watertight enclosures, and mounting fixtures.
- Electrical: power distribution, cabling, and sealing components.

## Software Stack

- Mission orchestration: `brain`
- Launch and shared configuration: `manta_bringup`
- Perception: `cv`
- State estimation and mapping: `ekfslam`
- Task execution and control: `tasks`
- Shared messages and services: `interfaces`
- MAVROS bridge and utilities: `mavros_control`

## Runtime Pipeline

1. `brain` selects the main or qualification sequence.
2. `cv` receives task commands and publishes feature observations.
3. `ekfslam` validates and repackages those observations.
4. `tasks` consumes the filtered stream and publishes RC overrides.
5. `brain` advances through the mission until completion.

## Build and Run

From the repository root:

```bash
colcon build
source install/setup.bash
```

Vehicle bringup:

```bash
ros2 launch manta_bringup robot_bringup.launch.py
```

Main mission:

```bash
ros2 launch manta_bringup main.launch.py
```

Qualification mission:

```bash
ros2 launch manta_bringup qual.launch.py
```

## Documentation

- Architecture: [docs/architecture.md](docs/architecture.md)
- Package docs: `src/*/README.md`
