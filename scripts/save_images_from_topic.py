import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import re
import cv2
import os
from rclpy.qos import QoSProfile, QoSReliabilityPolicy
import sys

class ImageSubscriber(Node):
    def __init__(self, save_directory=None, use_exposure_callback=False, exposure_ev=-1.0):
        super().__init__('image_subscriber')
        qos_profile = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            history=rclpy.qos.HistoryPolicy.KEEP_LAST,
            depth=10
        )

        self.bridge = CvBridge()
        self.use_exposure_callback = use_exposure_callback
        self.exposure_ev = exposure_ev

        callback = self.exposure_callback if self.use_exposure_callback else self.listener_callback
        self.subscription = self.create_subscription(
            Image,
            'camera/img',
            callback,
            qos_profile
        )
        self.subscription  # prevent unused variable warning

        if self.use_exposure_callback:
            self.publisher = self.create_publisher(Image, 'camera/img/dimmed', qos_profile)
            self.get_logger().info(
                f"Exposure callback enabled. Republishing reduced-exposure images to: camera/img/dimmed (ev={self.exposure_ev})"
            )
        else:
            if save_directory is None:
                raise ValueError("save_directory must be provided when use_exposure_callback is False")

            self.save_directory = os.path.join('bags', save_directory)
            os.makedirs(self.save_directory, exist_ok=True)
            self.get_logger().info(f"Images will be saved to: {self.save_directory}")

            # Determine the starting index for image naming (supports img_0001.jpg and img_1.jpg)
            existing_files = [f for f in os.listdir(self.save_directory) if re.match(r'^img_?\d+\.jpg$', f, re.IGNORECASE)]
            existing_indices = [int(re.search(r"(\d+)", f).group(1)) for f in existing_files]
            self.image_index = max(existing_indices) + 1 if existing_indices else 1

    def listener_callback(self, msg):
        try:
            # Convert ROS Image message to OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
            
            # Generate a unique filename
            img_name = f"img_{self.image_index:04d}.jpg"
            img_path = os.path.join(self.save_directory, img_name)

            # Save the image
            cv2.imwrite(img_path, cv_image)
            self.get_logger().info(f"Saved image: {img_path}")

            # Increment the image index
            self.image_index += 1
        except Exception as e:
            self.get_logger().error(f"Failed to save image: {e}")

    def exposure_callback(self, msg):
        try:
            # Convert ROS Image message to OpenCV image
            cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

            # Approximate exposure compensation in linear-light space using EV stops.
            # exposure_factor = 2^EV, so negative EV reduces exposure.
            exposure_factor = 2.0 ** self.exposure_ev
            srgb = cv_image.astype('float32') / 255.0
            linear = srgb ** 2.2
            adjusted_linear = linear * exposure_factor
            adjusted_srgb = adjusted_linear ** (1.0 / 2.2)
            dimmed = (adjusted_srgb.clip(0.0, 1.0) * 255.0).astype('uint8')

            # Convert back to ROS Image and preserve incoming message metadata.
            dimmed_msg = self.bridge.cv2_to_imgmsg(dimmed, encoding='bgr8')
            dimmed_msg.header = msg.header

            self.publisher.publish(dimmed_msg)
            self.get_logger().info("Published reduced-exposure image.")
        except Exception as e:
            self.get_logger().error(f"Failed to reduce exposure and publish image: {e}")

def main(args=None):
    rclpy.init(args=args)

    if len(sys.argv) != 2:
        print("Usage: python3 save_images_from_topic.py <directory_name> | --dim")
        sys.exit(1)

    arg = sys.argv[1]
    use_exposure_callback = arg == '--dim'
    save_directory = None if use_exposure_callback else arg

    image_subscriber = ImageSubscriber(
        save_directory,
        use_exposure_callback=use_exposure_callback,
    )

    try:
        rclpy.spin(image_subscriber)
    except KeyboardInterrupt:
        image_subscriber.get_logger().info('Shutting down image subscriber node.')
    finally:
        image_subscriber.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()