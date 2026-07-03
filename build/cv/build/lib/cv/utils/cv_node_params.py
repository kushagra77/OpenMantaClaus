from dataclasses import dataclass
import math
from typing import List

import cv2
import numpy as np
from rclpy.node import Node

DEFAULT_CAMERA_MATRIX = [
    516.363150,
    0.0,
    322.031903,
    0.0,
    517.166678,
    155.100222,
    0.0,
    0.0,
    1.0,
]

DEFAULT_DIST_COEFFS = [0.096618, -0.729662, -0.005456, -0.003541, 1.410276]

DEFAULT_FEATURE_NAMES = [
    "flag",
    "gate_left",
    "gate_right",
    "flare_1",
    "flare_2",
    "flare_3",
    "bucket_1",
    "bucket_2",
    "bucket_3",
    "bucket_4",
    "aruco_marker",
    "qual_gate_left",
    "qual_gate_right",
]

DEFAULT_YOLO_MODEL_PATH = "scripts/yolo/runs/detect/pretrain_generalist_v1/weights/best_saved_model/best_int8.tflite"


@dataclass(frozen=True)
class CVNodeParams:
    debug_flag: bool
    debug_image: bool
    debug_yolo: bool
    debug_fps_period_s: float
    default_task: str
    feature_names: List[str]
    yolo_model_path: str
    yolo_num_threads: int
    conf_threshold: float
    yolo_input_width: int
    yolo_input_height: int
    capture_rate_hz: float
    inactive_heartbeat_s: float
    capture_device_index: int
    capture_backend: int
    capture_frame_width: int
    capture_frame_height: int
    capture_fps: float
    capture_autofocus: int
    capture_auto_white_balance: int
    capture_focus: int
    capture_white_balance_blue_u: int
    capture_backlight: int
    capture_fourcc: str
    enable_auto_exposure: int
    manual_override_exposure: int
    min_exposure: int
    max_exposure: int
    brightness: int
    target_brightness: float
    exposure_check_interval: int
    processing_width: int
    processing_height: int
    camera_matrix: np.ndarray
    dist_coeffs: np.ndarray
    aruco_marker_ids: List[int]
    red_lower: np.ndarray
    red_higher: np.ndarray
    blue_lower: np.ndarray
    blue_higher: np.ndarray
    yellow_lower: np.ndarray
    yellow_higher: np.ndarray
    erode_kernel_size: int
    flare_percentage_threshold: float
    bucket_percentage_threshold: float
    base_std: float


def declare_cv_node_parameters(node: Node) -> None:
    node.declare_parameter("debug_flag", True)
    node.declare_parameter("debug_image", False)
    node.declare_parameter("debug_yolo", False)
    node.declare_parameter("debug_fps_period_s", 4.0)
    node.declare_parameter("default_task", "flag")
    node.declare_parameter("common.feature_names", DEFAULT_FEATURE_NAMES)
    node.declare_parameter("yolo.model_path", DEFAULT_YOLO_MODEL_PATH)
    node.declare_parameter("yolo.num_threads", 2)
    node.declare_parameter("yolo.conf_threshold", 0.8)
    node.declare_parameter("yolo.input_width", 256)
    node.declare_parameter("yolo.input_height", 256)

    node.declare_parameter("capture.device_index", 0)
    node.declare_parameter("capture.backend", int(cv2.CAP_V4L2))
    node.declare_parameter("capture.timer_rate_hz", 60.0)
    node.declare_parameter("capture.inactive_heartbeat_s", 0.5)
    node.declare_parameter("capture.frame_width", 640)
    node.declare_parameter("capture.frame_height", 360)
    node.declare_parameter("capture.processing_width", 640)
    node.declare_parameter("capture.processing_height", 360)
    node.declare_parameter("capture.fps", 60.0)
    node.declare_parameter("capture.autofocus", 0)
    node.declare_parameter("capture.auto_white_balance", 0)
    node.declare_parameter("capture.focus", 255)
    node.declare_parameter("capture.white_balance_blue_u", 5500)
    node.declare_parameter("capture.backlight", 0)
    node.declare_parameter("capture.fourcc", "MJPG")
    node.declare_parameter("enable_auto_exposure", 0)
    node.declare_parameter("manual_override_exposure", 0)
    node.declare_parameter("min_exposure", 10)
    node.declare_parameter("max_exposure", 500)
    node.declare_parameter("brightness", 10)
    node.declare_parameter("target_brightness", 225.0)
    node.declare_parameter("exposure_check_interval", 5)

    node.declare_parameter("camera.camera_matrix", DEFAULT_CAMERA_MATRIX)
    node.declare_parameter("camera.distortion_coefficients", DEFAULT_DIST_COEFFS)
    # ARUCO marker dataset mapping used by the aruco processor: list of 6 marker IDs
    node.declare_parameter("aruco_marker_ids", [0, 1, 2, 3, 6, 7])
    # Color range parameters (YUV color space)
    node.declare_parameter("color_ranges.red_lower", [120, 0, 0])
    node.declare_parameter("color_ranges.red_higher", [179, 255, 255])
    node.declare_parameter("color_ranges.blue_lower", [105, 180, 0])
    node.declare_parameter("color_ranges.blue_higher", [130, 255, 255])
    node.declare_parameter("color_ranges.yellow_lower", [50, 0, 0])
    node.declare_parameter("color_ranges.yellow_higher", [85, 255, 255])
    # Detection filter parameters
    node.declare_parameter("detection_filter.erode_kernel_size", 3)
    node.declare_parameter("detection_filter.flare_percentage_threshold", 0.05)
    node.declare_parameter("detection_filter.bucket_percentage_threshold", 0.05)
    node.declare_parameter("detection_filter.base_std_deg", 0.14)


def _load_vector_param(
    node: Node,
    name: str,
    default: List[float],
    expected_size: int,
    dtype,
) -> np.ndarray:
    values = list(node.get_parameter(name).value)
    if len(values) != expected_size:
        node.get_logger().warn(f"Parameter {name} expected {expected_size} values, using defaults")
        values = default
    return np.array(values, dtype=dtype)


def _load_matrix3_param(node: Node, name: str, default: List[float], dtype) -> np.ndarray:
    values = list(node.get_parameter(name).value)
    if len(values) != 9:
        node.get_logger().warn(f"Parameter {name} expected 9 values, using defaults")
        values = default
    return np.array(values, dtype=dtype).reshape((3, 3))


def load_cv_node_parameters(node: Node) -> CVNodeParams:
    return CVNodeParams(
        debug_flag=bool(node.get_parameter("debug_flag").value),
        debug_image=bool(node.get_parameter("debug_image").value),
        debug_yolo=bool(node.get_parameter("debug_yolo").value),
        debug_fps_period_s=float(node.get_parameter("debug_fps_period_s").value),
        default_task=str(node.get_parameter("default_task").value),
        feature_names=list(node.get_parameter("common.feature_names").value),
        yolo_model_path=str(node.get_parameter("yolo.model_path").value),
        yolo_num_threads=int(node.get_parameter("yolo.num_threads").value),
        conf_threshold=float(node.get_parameter("yolo.conf_threshold").value),
        yolo_input_width=int(node.get_parameter("yolo.input_width").value),
        yolo_input_height=int(node.get_parameter("yolo.input_height").value),
        capture_rate_hz=float(node.get_parameter("capture.timer_rate_hz").value),
        inactive_heartbeat_s=float(node.get_parameter("capture.inactive_heartbeat_s").value),
        capture_device_index=int(node.get_parameter("capture.device_index").value),
        capture_backend=int(node.get_parameter("capture.backend").value),
        capture_frame_width=int(node.get_parameter("capture.frame_width").value),
        capture_frame_height=int(node.get_parameter("capture.frame_height").value),
        capture_fps=float(node.get_parameter("capture.fps").value),
        capture_autofocus=int(node.get_parameter("capture.autofocus").value),
        capture_auto_white_balance=int(node.get_parameter("capture.auto_white_balance").value),
        capture_focus=int(node.get_parameter("capture.focus").value),
        capture_white_balance_blue_u=int(node.get_parameter("capture.white_balance_blue_u").value),
        capture_backlight=int(node.get_parameter("capture.backlight").value),
        capture_fourcc=str(node.get_parameter("capture.fourcc").value),
        enable_auto_exposure=int(node.get_parameter("enable_auto_exposure").value),
        manual_override_exposure=int(node.get_parameter("manual_override_exposure").value),
        min_exposure=int(node.get_parameter("min_exposure").value),
        max_exposure=int(node.get_parameter("max_exposure").value),
        brightness=int(node.get_parameter("brightness").value),
        target_brightness=float(node.get_parameter("target_brightness").value),
        exposure_check_interval=int(node.get_parameter("exposure_check_interval").value),
        processing_width=int(node.get_parameter("capture.processing_width").value),
        processing_height=int(node.get_parameter("capture.processing_height").value),
        camera_matrix=_load_matrix3_param(node, "camera.camera_matrix", DEFAULT_CAMERA_MATRIX, np.float64),
        dist_coeffs=_load_vector_param(node, "camera.distortion_coefficients", DEFAULT_DIST_COEFFS, 5, np.float64),
        aruco_marker_ids=list(node.get_parameter("aruco_marker_ids").value),
        red_lower=_load_vector_param(node, "color_ranges.red_lower", [0, 0, 80], 3, np.int32),
        red_higher=_load_vector_param(node, "color_ranges.red_higher", [140, 160, 255], 3, np.int32),
        blue_lower=_load_vector_param(node, "color_ranges.blue_lower", [50, 150, 0], 3, np.int32),
        blue_higher=_load_vector_param(node, "color_ranges.blue_higher", [160, 255, 135], 3, np.int32),
        yellow_lower=_load_vector_param(node, "color_ranges.yellow_lower", [150, 100, 110], 3, np.int32),
        yellow_higher=_load_vector_param(node, "color_ranges.yellow_higher", [190, 150, 200], 3, np.int32),
        erode_kernel_size=int(node.get_parameter("detection_filter.erode_kernel_size").value),
        flare_percentage_threshold=float(node.get_parameter("detection_filter.flare_percentage_threshold").value),
        bucket_percentage_threshold=float(node.get_parameter("detection_filter.bucket_percentage_threshold").value),
        base_std=math.radians(float(node.get_parameter("detection_filter.base_std_deg").value)),
    )
