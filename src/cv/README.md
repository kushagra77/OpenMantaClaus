# cv

`cv` is the perception package. It captures camera frames, runs a single YOLO26n TFLite inference pipeline, and publishes feature observations for SLAM and task execution.

`cv` is the production runtime package. Perception changes are typically developed and validated first in `scripts/cv_testing`, then copied back here once they are stable.

## Purpose

- Acquire and preprocess camera images.
- Run YOLO inference based on the current task command.
- Publish raw `FeatureObservations` for downstream EKF filtering.
- Keep camera state and exposure control consistent across runtime and testing nodes.
- Serve as the mission-facing perception path used by `brain` and the main launch files.

## Executables

- `cv_node` (main runtime node for forward camera)
- `testing_node` (camera throughput/debug publisher)
- `bottom_cv_node` (bottom camera UDP bridge - experimental)
- `cv_replay_node` (offline replay/testing)

## Interfaces

- Publishes:
	- `/camera/img` (`sensor_msgs/msg/Image`)
	- `/cv/feature_observations` (`interfaces/msg/FeatureObservations`)
- Service server:
	- `/cv/task_command` (`interfaces/srv/TaskCommand`)
- Service client:
	- `/tasks/task_command` (`interfaces/srv/TaskCommand`)

Task handoff behavior:

- `brain` requests task changes through `/cv/task_command`.
- `cv` updates the current task inside the YOLO inference class and forwards task start/update to `tasks` using `/tasks/task_command`.
- `cv_node` now runs YOLO first, then publishes the debug image if `debug_image` is enabled.

## Parameters

Declared and loaded in:

- `src/cv/cv/utils/cv_node_params.py`

Shared camera setup and frame-processing logic lives in:

- `src/cv/cv/utils/camera_control.py`

YOLO inference and bounding-box post-processing live in:

- `src/cv/cv/yolo.py`

Detection filtering (task-specific feature extraction):

- `src/cv/cv/utils/filter_detections.py` - task-aware filtering of YOLO detections into bearing observations

### **YOLO Model and Inference**

- **Model**: TFLite quantized (`scripts/yolo/runs/detect/pretrain_generalist_v1/weights/best_saved_model/best_int8.tflite`)
- **Input size**: 256×256 pixels (letterboxed from original frame)
- **Detection classes** (4 total):
  - `flag` (id=0)
  - `gate` (id=1)
  - `flare` (id=2)
  - `bucket` (id=3)
- **Inference threads**: Configurable (default 2)
- **Confidence threshold**: 0.8 (configurable)
- **Framework**: TensorFlow Lite with Python interpreter

### **Detection Filtering Pipeline**

After YOLO inference, detections are task-aware filtered:

- **Gate filter** (`filter_gate()`): Splits detections by position to identify left/right posts
- **Flag filter** (`filter_flag()`): Extracts centroid and converts to bearing angle
- **Flare filter** (`filter_flare()`): Extracts centroid-based bearing, tags as generic flare feature
- **Bucket filter** (`filter_bucket()`): Extracts centroid-based bearing, tags as generic bucket feature

All bearing observations use camera intrinsics (fx=533.16, cx=318.89) to convert pixel coordinates to radians.

### **ArUco Detection**

- **Library**: OpenCV ArUco (4.7+)
- **Marker set**: IDs [0, 1, 2, 3, 6, 7]
- **Behavior**: Active during `aruco` task only
- **Output**: Publishes first detected marker's bearing as observation

### **Camera Exposure Parameters**

- `enable_auto_exposure` (0 or 1): Enable/disable camera auto exposure
- `manual_override_exposure`: Force fixed exposure value (>0 overrides auto/smart modes)
- `min_exposure`, `max_exposure`: Bounds for smart manual exposure mode
- `brightness`: Target image brightness for smart exposure
- `target_brightness`: Reference brightness level
- `exposure_check_interval`: How often to update smart exposure

Exposure mode behavior:

- `manual_override_exposure > 0`: force fixed manual exposure and ignore auto/smart bounds.
- `manual_override_exposure <= 0` and `enable_auto_exposure = 1`: use camera auto exposure.
- `manual_override_exposure <= 0` and `enable_auto_exposure = 0`: use smart manual exposure with min/max clamping.

Gain behavior:

- camera gain is fixed to `0` in code and is not a parameter.

### **YOLO and Frame Parameters**

- `yolo.model_path`: Path to TFLite model file (environment override: `CV_YOLO_MODEL_PATH`)
- `yolo.num_threads`: Number of inference threads (default 2)
- `yolo.conf_threshold`: YOLO confidence threshold (default 0.8)
- `capture.fps`: Frame capture rate (default 60 FPS)
- `capture.device_index`: Camera device index (default 0, use `V4L2` backend)
- `capture.frame_width`, `capture.frame_height`: Capture resolution (default 640×360)
- `processing_width`, `processing_height`: Processing resolution for YOLO (default 640×360)

### **ArUco Parameters**

- `aruco_marker_ids` (or `aruco.marker_ds`): Array of 6 integer ArUco marker IDs to detect. Parameter name supports both formats for backward compatibility.

Current runtime parameters include camera control, capture timing, debug flags, `debug_yolo` switch, YOLO configuration, and ArUco marker detection settings.

Current YOLO note:

- `debug_yolo=true` draws the YOLO bounding boxes on the original frame before the debug image is published.
- The YOLO model path defaults to the repository copy of `scripts/yolo/runs/detect/pretrain_generalist_v1/weights/best_saved_model/best_int8.tflite`, can be overridden with `yolo.model_path`, and can also be overridden with `CV_YOLO_MODEL_PATH`.

Current node behavior note:

- `cv_node` contains a playback branch (`self._playback`) but runtime launch is configured for live capture (`False`).
- when inactive (`task=none`), `cv_node` still processes one frame per second to keep camera preprocessing/exposure updates active.

## Usage

Build and source from workspace root:

```bash
colcon build --packages-select cv interfaces
source install/setup.bash
```

Run:

```bash
ros2 run cv cv_node
```

The main launches in `brain` start this package with the shared `params.yaml`.