# manta_bringup

`manta_bringup` owns the runtime launch files and shared configuration YAMLs for the MantaClaus stack.

## Purpose

- Start the robot bringup stack.
- Start the brain-only mission entry points.
- Hold shared parameters used by `cv`, `tasks`, and `ekfslam`.
- Own the MAVROS launch assets and shared MAVROS configuration.

## Launch Files

- `robot_bringup.launch.py`
- `main.launch.py`
- `qual.launch.py`
- `only_mavros.launch.py`
- `sim_only_mavros.launch.py`
- `hardcode_qual_full.launch.py`
- `mavros.launch`

`robot_bringup.launch.py` starts the perception, SLAM, and task nodes. `main.launch.py` and `qual.launch.py` only start `brain` with the main or qualification mission parameters.

## Shared Config

- `params.yaml`
- `brain_qual.yaml`
- `dummy_cv.yaml`
- `apm_config.yaml`
- `apm_pluginlist.yaml`

## Usage

Build and source from workspace root:

```bash
colcon build --packages-select manta_bringup
source install/setup.bash
```

Run the vehicle bringup flow:

```bash
ros2 launch manta_bringup robot_bringup.launch.py
```

Run the mission orchestrator:

```bash
ros2 launch manta_bringup main.launch.py
ros2 launch manta_bringup qual.launch.py
```
