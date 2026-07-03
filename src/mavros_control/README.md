# mavros_control

`mavros_control` provides the MAVROS launch/config bridge and a Python controller utility node used for integration and manual testing.

## Purpose in This Repository

- Offer a standalone `controller` executable for low-level movement and test workflows.

## Executable

- `controller` (`mavros_control/controller.py`)

## Launch Files

- `launch/mavros.launch` (included by `manta_bringup` launches)
- `launch/demo.launch.py` (controller demo)

## Usage

Build and source from workspace root:

```bash
colcon build --packages-select mavros_control
source install/setup.bash
```

Run demo:

```bash
ros2 launch mavros_control demo.launch.py
```

In normal mission runs, MAVROS is started indirectly via:

```bash
ros2 launch manta_bringup robot_bringup.launch.py
```