import time

import cv2
import rclpy
from cv_bridge import CvBridge
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image

from .constants import CAMERA_FRAME_ID
from .utils.camera_control import CameraControl
from .utils.cv_node_params import declare_cv_node_parameters, load_cv_node_parameters


class TestingNode(Node):
    def __init__(self) -> None:
        super().__init__("testing_node")
        declare_cv_node_parameters(self)
        params = load_cv_node_parameters(self)

        self._capture_rate_hz = params.capture_rate_hz

        self._cap = cv2.VideoCapture(
            params.capture_device_index,
            params.capture_backend,
        )
        if not self._cap.isOpened():
            self.get_logger().error("Cannot open camera")

        self._camera_control = CameraControl(self._cap, params)
        self._camera_control.setup_camera()
        self._bridge = CvBridge()
        self._image_pub = self.create_publisher(Image, "/camera/img", qos_profile_sensor_data)

        self._frame_count = 0
        self._last_log = time.time()
        self._timer = self.create_timer(1.0 / self._capture_rate_hz, self._capture_loop)
        

    def _capture_loop(self) -> None:
        if not self._cap.isOpened():
            return

        ok, frame = self._cap.read()
        if not ok:
            return

        frame = self._camera_control.process_frame(frame)

        msg = self._bridge.cv2_to_imgmsg(frame, encoding="bgr8")
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = CAMERA_FRAME_ID
        self._image_pub.publish(msg)

        self._frame_count += 1
        now = time.time()
        elapsed = now - self._last_log
        if elapsed >= 1.0:
            self.get_logger().info(f"Actual Publishing FPS: {self._frame_count / elapsed:.2f}")
            self._frame_count = 0
            self._last_log = now

    def _dummy_image_callback(self, _msg: Image) -> None:
        frame = self._camera_control.process_frame(self._bridge.imgmsg_to_cv2(_msg, desired_encoding="bgr8"))

        msg = self._bridge.cv2_to_imgmsg(frame, encoding="bgr8")
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = CAMERA_FRAME_ID
        self._image_pub.publish(msg)

    def destroy_node(self) -> bool:
        if self._cap.isOpened():
            self._cap.release()
        return super().destroy_node()


def main(args=None) -> None:
    rclpy.init(args=args)
    node = TestingNode()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()
