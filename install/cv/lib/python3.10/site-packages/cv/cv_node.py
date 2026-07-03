from typing import List, Sequence, Tuple

import cv2
import numpy as np
import rclpy
from cv_bridge import CvBridge
from interfaces.msg import FeatureObservation, FeatureObservations
from interfaces.srv import TaskCommand
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image
from std_msgs.msg import Int32, String, Header

from .utils.camera_control import CameraControl
from .constants import CAMERA_FRAME_ID
from .utils.cv_node_params import declare_cv_node_parameters, load_cv_node_parameters
from .utils.filter_detections import DetectionFilter, CLASS_NAMES
from .utils.yolo import Yolo

class CVNode(Node):
    def __init__(self) -> None:
        """Initialize the CV node.

        Sets up parameters, camera control, YOLO inference engine, publishers,
        and the single-threaded capture/inference flow.
        """
        super().__init__("cv_node")
        declare_cv_node_parameters(self)
        params = load_cv_node_parameters(self)

        self._playback = False
        self._debug_flag = params.debug_flag
        self._debug_image = params.debug_image
        self._debug_yolo = params.debug_yolo
        self._debug_fps_period_s = params.debug_fps_period_s
        self._default_task = params.default_task
        self._feature_names = params.feature_names
        self._yolo_model_path = params.yolo_model_path
        self._yolo_num_threads = params.yolo_num_threads
        self._conf_threshold = params.conf_threshold
        self._yolo_input_width = params.yolo_input_width
        self._yolo_input_height = params.yolo_input_height
        self._capture_rate_hz = params.capture_rate_hz
        self._inactive_heartbeat_s = params.inactive_heartbeat_s
        self._aruco_marker_ids = params.aruco_marker_ids

        self._fps_window_start_ns = self.get_clock().now().nanoseconds
        self._frame_count = 0.0
        
        self._camera_fx = 533.16473
        self._camera_cx = 318.89810

        self._cap = cv2.VideoCapture(
            params.capture_device_index,
            params.capture_backend,
        )
        if not self._cap.isOpened():
            self.get_logger().error("Failed to open camera")
        if self._playback:
            self.get_logger().warn("WARNING!! PLAYBACK MODE IS ENABLED")
            
        self._active = False
        self._aruco = False
        self._last_inactive_process_ns = 0
        self._bridge = CvBridge()
        
        # --- Initialize Detection Filter ---
        self._detection_filter = DetectionFilter(
            camera_fx=self._camera_fx,
            camera_cx=self._camera_cx,
            feature_names=params.feature_names,
            red_lower=list(params.red_lower),
            red_higher=list(params.red_higher),
            blue_lower=list(params.blue_lower),
            blue_higher=list(params.blue_higher),
            yellow_lower=list(params.yellow_lower),
            yellow_higher=list(params.yellow_higher),
            erode_kernel_size=params.erode_kernel_size,
            flare_percentage_threshold=params.flare_percentage_threshold,
            bucket_percentage_threshold=params.bucket_percentage_threshold,
            input_width=self._yolo_input_width,
            input_height=self._yolo_input_height,
            conf_threshold=params.conf_threshold,
            curr_task=params.default_task,
        )
        
        self._yolo = Yolo(model_path=self._yolo_model_path, num_threads=self._yolo_num_threads)
        
        input_details, output_details = self._yolo.get_details()
        self._yolo_input_dtype = input_details["dtype"]
        self._yolo_input_scale, self._yolo_input_zp = input_details["quantization"]
        self._yolo_output_scale, self._yolo_output_zp = output_details["quantization"]

        self._img_size = (params.processing_width, params.processing_height)
        self._camera_control = CameraControl(self._cap, params)
        self._camera_control.setup_camera()

        if self._debug_image:
            self._image_pub = self.create_publisher(Image, "/camera/debug_img", qos_profile_sensor_data)
        if self._debug_flag:
            self._debug_pub = self.create_publisher(String, "/debug", 10)
            
        self._feature_pub = self.create_publisher(FeatureObservations, "/cv/feature_observations", 10)
        # Publisher to notify when a camera frame is triggered/processed
        self._frame_trigger_pub = self.create_publisher(Header, "camera/frame_trigger", 10)
        self._task_command_server = self.create_service(
            TaskCommand, "/cv/task_command", self._task_command_callback
        )
        self._task_command_client = self.create_client(
            TaskCommand, "/tasks/task_command"
        )
        
        self.bottom_camera_publisher = self.create_publisher(Int32, '/bottom_camera/command', 10)

        
        self._timer = self.create_timer(1.0 / self._capture_rate_hz, self._capture_frame)
        if self._playback:
            self.sub = self.create_subscription(
                Image, "/camera/debug_img", self.playback_callback, qos_profile_sensor_data
            )
        self._fps_timer = None
        self.cur_playback_img = None
        self.cur_playback_img_timestamp = None
        if self._debug_flag:
            self._fps_timer = self.create_timer(self._debug_fps_period_s, self._log_fps)
            
    def playback_callback(self, msg):
        self.cur_playback_img = self._bridge.imgmsg_to_cv2(msg, desired_encoding="bgr8")
        self.cur_playback_img_timestamp = msg.header.stamp
        
    def _preprocess_frame(self, frame: np.ndarray) -> Tuple[np.ndarray, float, int, int]:
        """Prepare a BGR frame for YOLO inference.

        Steps:
        - Letterbox/resize to model input (256x256) while preserving aspect ratio.
        - Pad with gray (114) to fill the model input size.
        - Convert BGR->RGB and normalize to [0,1].
        - Apply quantization transform based on interpreter input scale/zero point
          (if model is quantized). Returns the batched input tensor and
          the scale/pad metadata needed to unpad and unscale detections later.
        """
        expected_w, expected_h = self._yolo_input_width, self._yolo_input_height
        original_h, original_w = frame.shape[:2]
        scale = min(expected_w / original_w, expected_h / original_h)
        resized_w = int(original_w * scale)
        resized_h = int(original_h * scale)

        resized_frame = cv2.resize(frame, (resized_w, resized_h), interpolation=cv2.INTER_LINEAR)
        padded_frame = np.full((expected_h, expected_w, 3), 114, dtype=np.uint8)
        pad_w = (expected_w - resized_w) // 2
        pad_h = (expected_h - resized_h) // 2
        padded_frame[pad_h : pad_h + resized_h, pad_w : pad_w + resized_w] = resized_frame

        # Convert color space for most YOLO models which expect RGB input
        frame_rgb = cv2.cvtColor(padded_frame, cv2.COLOR_BGR2RGB)
        # Normalize pixel values to [0, 1] floating point
        frame_normalized = frame_rgb.astype(np.float32) / 255.0

        if self._yolo_input_scale > 0:
            if self._yolo_input_dtype == np.int8:
                input_data = np.clip(
                    np.round(frame_normalized / self._yolo_input_scale + self._yolo_input_zp),
                    -128,
                    127,
                ).astype(np.int8)
            else:
                input_data = np.clip(
                    np.round(frame_normalized / self._yolo_input_scale + self._yolo_input_zp),
                    0,
                    255,
                ).astype(np.uint8)
        else:
            input_data = frame_normalized

        # Batch dimension required by the interpreter
        input_data = np.expand_dims(input_data, axis=0)
        # Return input tensor + metadata to reverse preprocessing
        return input_data, scale, pad_w, pad_h

    def yolo_inference(self, frame: np.ndarray) -> List[dict]:
        """Run preprocessing, inference, and box post-processing for one frame.

        Returns the filtered detections.
        """
        input_data, scale, pad_w, pad_h = self._preprocess_frame(frame)

        raw_output = self._yolo.infer(input_data)

        if self._yolo_output_scale > 0:
            predictions = (raw_output.astype(np.float32) - self._yolo_output_zp) * self._yolo_output_scale
        else:
            predictions = raw_output

        detections = self._detection_filter._filter_and_transform_boxes(
            predictions[0], scale, pad_w, pad_h, frame.shape[:2]
        )
        self._detection_filter.set_latest_raw_image(frame)

        return detections

    def _capture_frame(self) -> None:
        
        if self._playback:
            if self.cur_playback_img is not None:
                frame = self.cur_playback_img
                self.cur_playback_img = None
            else:
                return
        else:
            if not self._cap.isOpened():
                return
            # In inactive mode, throttle processing to a low-rate camera heartbeat.
            if not self._active:
                now_ns = self.get_clock().now().nanoseconds
                if now_ns - self._last_inactive_process_ns < int(self._inactive_heartbeat_s * 1_000_000_000):
                    return
                self._last_inactive_process_ns = now_ns
                
            ok, frame = self._cap.read()
            if not ok:
                self.get_logger().error("Failed to capture frame")
                return
            frame = self._camera_control.process_frame(frame)


        if not self._active:
            if self._debug_image:
                self.get_logger().info('Camera inactive, publishing debug image anyway.')
                image_msg = self._bridge.cv2_to_imgmsg(frame, encoding="bgr8")
                image_msg.header.stamp = self.get_clock().now().to_msg()
                image_msg.header.frame_id = CAMERA_FRAME_ID
                self._image_pub.publish(image_msg)
            return
        
        # publish frame trigger timestamp
        hdr = Header()
        hdr.stamp = self.get_clock().now().to_msg()
        if self._playback:
            hdr.stamp = self.cur_playback_img_timestamp
        hdr.frame_id = CAMERA_FRAME_ID
        self._frame_trigger_pub.publish(hdr)
        
        cur_task, _ = self._detection_filter.get_task_state()
        
        # toggle aruco mode at every frame during aruco task
        if cur_task == "aruco":
            self._aruco = not self._aruco
            
        if self._aruco:
            self.process_aruco(frame, hdr)
            return
        
        self.yolo_inference(frame)
        
        feature_msg = self._detection_filter._detections_to_observations()
        feature_msg.header = hdr
        feature_msg.header.frame_id = CAMERA_FRAME_ID
        self._feature_pub.publish(feature_msg)

        if self._debug_image:
            if self._debug_yolo:
                frame = self._detection_filter._draw_boxes()
            image_msg = self._bridge.cv2_to_imgmsg(frame, encoding="bgr8")
            image_msg.header.stamp = self.get_clock().now().to_msg()
            image_msg.header.frame_id = CAMERA_FRAME_ID
            self._image_pub.publish(image_msg)

        if self._debug_flag:
            self._frame_count += 1.0
            self._publish_observation_debug(feature_msg.observations)
            
    def process_aruco(self, frame: np.ndarray, hdr: Header) -> None:
        aruco_dict = cv2.aruco.getPredefinedDictionary(cv2.aruco.DICT_4X4_50)
        parameters = cv2.aruco.DetectorParameters()
        detector = cv2.aruco.ArucoDetector(aruco_dict, parameters)
        
        corners, ids, _ = detector.detectMarkers(frame)
        
        # choose the first found marker
        marker_id = -1
        if ids is not None and len(ids) > 0:
            marker_id = int(ids[0][0])
        else:
            self._publish_debug("aruco", "no markers detected")
            return
        
        if marker_id in self._aruco_marker_ids:
            marker_id = self._aruco_marker_ids.index(marker_id)
        else:
            return

        cx = np.mean(corners[0][0, :, 0])
        obs = self._detection_filter._get_obs_from_x(cx)
        obs.color = marker_id
        obs.confident = True
        obs.bearing_cov = np.deg2rad(0.01)  # 0.01 degree covariance in radians
        obs.id = self._feature_names.index("aruco_marker") if "aruco_marker" in self._feature_names else -1
        feature_msg = FeatureObservations()
        feature_msg.header = hdr
        feature_msg.observations = [obs]
        self._feature_pub.publish(feature_msg)
        self._publish_debug("aruco", f"detected marker_id={marker_id} bearing={obs.bearing:.6f}")
        self.get_logger().info(f'aruco marker detected id={marker_id}')
        
        

    def _publish_debug(self, event: str, detail: str) -> None:
        """Publish a short debug string when debug_flag is enabled."""
        if not self._debug_flag:
            return
        cur_task, _ = self._detection_filter.get_task_state()
        msg = String()
        msg.data = f"source=cv/cv_node event={event} task={cur_task} {detail}"
        self._debug_pub.publish(msg)

    def _publish_observation_debug(self, observations) -> None:
        feature_parts = []
        for obs in observations:
            feature_name = self._feature_names[obs.id] if obs.id >= 0 and obs.id < len(self._feature_names) else f"id_{obs.id}"
            feature_parts.append(f"{feature_name}(id={obs.id},bearing={obs.bearing:.6f})")

        seen = ",".join(feature_parts) if feature_parts else "none"
        self._publish_debug(
            "observations",
            f"count={len(observations)} seen=[{seen}]",
        )

    def _log_fps(self) -> None:
        now_ns = self.get_clock().now().nanoseconds
        elapsed_s = max((now_ns - self._fps_window_start_ns) / 1_000_000_000.0, 1e-9)
        fps = self._frame_count / elapsed_s
        self._publish_debug("fps", f"frame_fps={fps:.2f} elapsed_s={elapsed_s:.2f}")
        self._fps_window_start_ns = now_ns
        self._frame_count = 0.0

    def _task_command_callback(self, request: TaskCommand.Request, response: TaskCommand.Response) -> TaskCommand.Response:
        self.get_logger().info(f"Received task command: {request.command}")
        cur_task, _ = self._detection_filter.get_task_state()
        if cur_task == "gripper":
            self.bottom_camera_publisher.publish(Int32(data=0)) # disable the bottom camera
            
        self._detection_filter.set_task_state(request.command, bool(request.initial))
        self._aruco = False
        self._active = True
        response.success = True

        if request.command == "none":
            self._active = False
        elif request.command == "qual_gate" and not request.initial:
            # qual gate return doesn't trigger the task node again
            return response
        elif request.command == "gripper":
            self._active = False
            if request.initial:
                self.bottom_camera_publisher.publish(Int32(data=1)) # enable putdown
            else:
                self.bottom_camera_publisher.publish(Int32(data=2)) # enable pickup

        req = TaskCommand.Request()
        req.command = request.command
        req.initial = request.initial
        if self._task_command_client.service_is_ready():
            self._task_command_client.call_async(req)
        else:
            self.get_logger().warn("Task command service /tasks/task_command not available, cannot forward command")
            
        return response

    def destroy_node(self) -> bool:
        if self._cap.isOpened():
            self._cap.release()
        return super().destroy_node()


def main(args=None) -> None:
    rclpy.init(args=args)
    node = CVNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()