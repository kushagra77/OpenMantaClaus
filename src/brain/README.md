# brain

`brain` is the mission-orchestration package. It performs vehicle setup, dispatches task commands, and advances mission state when tasks report completion.

## Purpose

- Own startup sequencing for the run.
- Select mission sequence (`main` vs `qualification`).
- Start and update tasks by calling `cv`, which then calls `tasks`.
- On mission completion, send `none` to `cv` before resurfacing to stop CV/task activity.

## Main Node

- Executable: `brain_node`
- Source: `src/brain/src/brain.cpp`

## Interfaces

- Subscribes:
	- `mavros/state` (`mavros_msgs/msg/State`)
	- `/tasks/task_status` (`interfaces/msg/TaskStatus`)
- Publishes:
	- `mavros/setpoint_position/global` (`geographic_msgs/msg/GeoPoseStamped`)
- Service clients:
	- `mavros/cmd/arming` (`mavros_msgs/srv/CommandBool`)
	- `mavros/set_mode` (`mavros_msgs/srv/SetMode`)
	- `/cv/task_command` (`interfaces/srv/TaskCommand`)
- Service server:
	- `/tasks/task_complete` (`interfaces/srv/TaskComplete`)

Task start/update path in current runtime:

1. `brain` calls `/cv/task_command`
2. `cv` calls `/tasks/task_command` for normal start/update requests
3. for `qual_gate` with `initial=false`, `cv` updates detector mode locally and does not forward that specific update to `tasks`

## SAUVC Qualification Behavior

For qualification runs, the queue is configured as `["qual_gate", "-qual_gate"]`.

`qual_gate` task logic internally performs:

1. Forward gate pass
2. U-turn
3. Reverse gate pass

`-qual_gate` is still sent intentionally so the `cv` package can switch to reverse-direction qual-gate detector behavior during the return segment.

## Parameters

Loaded from `src/manta_bringup/launch/params.yaml` under:

- **Mission configuration** (`mission.*`):
  - `main_run`: true/false - selects main arena sequence (true) or qualification sequence (false)
  - `main_sequence`: list of task names for main mission (arena run)
  - `qual_sequence`: list of task names for qualification run
- **Setup configuration** (`setup.*`):
  - `target_depth_m`: Target water depth for mission (default 0.6m)
  - `start_delay_s`: Delay before starting mission sequence (default 15 seconds)

## Usage

Build and source from workspace root:

```bash
colcon build --packages-select brain interfaces
source install/setup.bash
```

Run directly:

```bash
ros2 run brain brain_node
```

Normally this package is started through:

```bash
ros2 launch manta_bringup robot_bringup.launch.py
ros2 launch manta_bringup main.launch.py
ros2 launch manta_bringup qual.launch.py
```
