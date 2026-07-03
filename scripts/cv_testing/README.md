# cv_testing

`cv_testing` is a lightweight image replay publisher used to stream frames from a folder directly to `/camera/img/`.

## Features

- Reads image files (`.jpg`, `.jpeg`, `.png`) from a directory.
- Sorts `img_<number>` files numerically when available and loops continuously.
- Publishes raw image frames as `sensor_msgs/msg/Image` on `/camera/img/`.
- Does not perform any CV detection, annotation, or post-processing.

## Run

```bash
colcon build --packages-select cv_testing
source install/setup.bash
ros2 run cv_testing cv_testing_node
```

## Configure

Edit constants in `cv_testing/cv_testing_node.py`:

- `INPUT_IMAGE_DIR`: directory containing images to replay.
- `DIRECTORY_RATE_HZ`: publish rate for replay.
- `OUTPUT_TOPIC`: output image topic (default `/camera/img/`).
