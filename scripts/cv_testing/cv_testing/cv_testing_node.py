import rclpy
from cv_bridge import CvBridge
from rclpy.node import Node
from sensor_msgs.msg import Image

from .utils.image_sequence import ImageSequence


# Editable hardcoded settings for quick bag testing.
INPUT_IMAGE_DIR = "bags/gatequaltough/"
DIRECTORY_RATE_HZ = 20.0
OUTPUT_TOPIC = "/camera/img/"


class CVTestingNode(Node):
    def __init__(self) -> None:
        super().__init__("cv_testing_node")

        self._bridge = CvBridge()

        self._image_pub = self.create_publisher(Image, OUTPUT_TOPIC, 10)
        self._image_sequence = ImageSequence(INPUT_IMAGE_DIR)
        self._timer = self.create_timer(1.0 / DIRECTORY_RATE_HZ, self._timer_callback)

        self.get_logger().info(
            f"cv_testing_node publishing replayed images from {INPUT_IMAGE_DIR} to {OUTPUT_TOPIC} at {DIRECTORY_RATE_HZ:.2f} Hz"
        )

    def _timer_callback(self) -> None:
        image_path, frame = self._image_sequence.next_image()
        msg = self._bridge.cv2_to_imgmsg(frame, encoding="bgr8")
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "manta_camera"
        self._image_pub.publish(msg)
        self.get_logger().debug(f"Published {image_path.name} to {OUTPUT_TOPIC}")


def main(args=None) -> None:
    rclpy.init(args=args)
    node = CVTestingNode()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()
