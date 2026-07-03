import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2
import time
import numpy as np
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

class SmartCameraCaptureNode(Node):
    def __init__(self):
        super().__init__('smart_camera_capture')
        
        qos_profile = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=1
        )
        
        self.bridge = CvBridge()
        self.publisher_ = self.create_publisher(Image, 'camera/image_raw', qos_profile)
        
        self.get_logger().info("Attempting to open camera device /dev/video0...")
        self.cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
        if not self.cap.isOpened():
            self.get_logger().error("CRITICAL: Could not open video device.")
            rclpy.shutdown()
            return

        # --- Smart Exposure & Capture State Variables ---
        self.current_exposure = 20         
        
        self.frame_counter = 0             
        self.exposure_check_interval = 5  
        
        self.images_saved = 0
        self.max_images = 5
        self.save_interval = 3.0           
        self.last_save_time = None
        self.count = 0                     
        
        # FPS Tracking
        self.frames_since_last_save = 0
        
        self.setup_camera_360p()
        
        self.timer = self.create_timer(1.0/120.0, self.timer_callback)
        self.get_logger().info("--------------------------------------------------")
        self.get_logger().info("Starting node. Warming up for 40 frames...")
        self.get_logger().info("--------------------------------------------------")

    def verify_setting(self, prop_id, prop_name, target_value):
        self.cap.set(prop_id, target_value)
        time.sleep(0.05) 
        actual_value = self.cap.get(prop_id)
        
        if abs(actual_value - target_value) < 0.1:
            self.get_logger().info(f"[OK] {prop_name}: {actual_value}")
            return True
        else:
            self.get_logger().error(f"[FAIL] {prop_name}: Requested {target_value}, Got {actual_value}")
            return False

    def setup_camera_360p(self):
        self.get_logger().info("--- Configuring Camera Settings ---")
        
        self.cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
        
        self.verify_setting(cv2.CAP_PROP_FRAME_WIDTH, "Width", 640)
        self.verify_setting(cv2.CAP_PROP_FRAME_HEIGHT, "Height", 360)
        self.verify_setting(cv2.CAP_PROP_FPS, "FPS", 60)

        self.verify_setting(cv2.CAP_PROP_AUTO_EXPOSURE, "Auto Exposure", 1)
        self.verify_setting(cv2.CAP_PROP_EXPOSURE, "Exposure Time", self.current_exposure)
        # Lock Gain to 0 permanently
        self.verify_setting(cv2.CAP_PROP_GAIN, "Gain", 0)
        self.verify_setting(cv2.CAP_PROP_BRIGHTNESS, "Brightness", 10)
        self.verify_setting(cv2.CAP_PROP_BACKLIGHT, "Backlight Comp", 0)

        self.verify_setting(cv2.CAP_PROP_AUTOFOCUS, "Autofocus", 0)
        self.verify_setting(cv2.CAP_PROP_FOCUS, "Focus", 255)
        self.verify_setting(cv2.CAP_PROP_AUTO_WB, "Auto WB", 0)
        self.verify_setting(cv2.CAP_PROP_WB_TEMPERATURE, "WB Temp", 5500)

        self.get_logger().info("--- Configuration Complete ---")

    def optimize_exposure(self, frame):
        """Single-Tier Controller (Exposure Only)"""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        p90 = np.percentile(gray, 90)
        p95 = np.percentile(gray, 95)
        
        highlight_pixels = gray[(gray >= p90) & (gray <= p95)]
        
        if len(highlight_pixels) > 0:
            highlight_brightness = np.mean(highlight_pixels)
        else:
            highlight_brightness = p95
            
        target_brightness = 235.0
        error = target_brightness - highlight_brightness
        
        if abs(error) > 10:
            Kp = 0.9 
            step = int(error * Kp)
            
            self.current_exposure += step
            
            # Clamp Exposure to safe hardware limits
            max_exposure_limit = 500
            min_exposure_limit = 15
            
            self.current_exposure = max(min_exposure_limit, min(max_exposure_limit, self.current_exposure))
            
            # Send to hardware
            self.cap.set(cv2.CAP_PROP_EXPOSURE, self.current_exposure)

    def timer_callback(self):
        ret, frame = self.cap.read()
        now = time.time()

        if ret:
            # Phase 1: Warmup
            if self.count < 40:
                self.count += 1
                if self.count == 40:
                    self.get_logger().info("Warmup complete. Starting capture sequence...")
                    self.last_save_time = now 
                    self.frames_since_last_save = 0
                return

            self.frame_counter += 1
            self.frames_since_last_save += 1

            # Phase 2: Run Smart Exposure Logic
            if self.frame_counter % self.exposure_check_interval == 0:
                self.optimize_exposure(frame)

            # Phase 3: Interval Saving
            elapsed_since_save = now - self.last_save_time
            if elapsed_since_save >= self.save_interval:
                self.images_saved += 1
                
                interval_fps = self.frames_since_last_save / elapsed_since_save
                
                output_path = f"bags/smart_capture_{self.images_saved}.jpg"
                saved = cv2.imwrite(output_path, frame)

                if saved:
                    # Log removed dynamic gain metric
                    self.get_logger().info(
                        f"[{self.images_saved}/{self.max_images}] Saved {output_path} | "
                        f"Exp: {self.current_exposure} | "
                        f"FPS: {interval_fps:.1f}"
                    )
                else:
                    self.get_logger().error(f"Failed to save {output_path}")

                self.last_save_time = now
                self.frames_since_last_save = 0

                # Phase 4: Completion
                if self.images_saved >= self.max_images:
                    self.get_logger().info("--------------------------------------------------")
                    self.get_logger().info("All 5 images captured. Shutting down cleanly.")
                    self.get_logger().info("--------------------------------------------------")
                    self.timer.cancel()
                    rclpy.shutdown()

def main(args=None):
    rclpy.init(args=args)
    node = SmartCameraCaptureNode()
    
    if rclpy.ok():
        try:
            rclpy.spin(node)
        except KeyboardInterrupt:
            pass
        finally:
            if hasattr(node, 'cap') and node.cap.isOpened():
                node.cap.release()
            try:
                node.destroy_node()
            except Exception:
                pass
            if rclpy.ok():
                rclpy.shutdown()

if __name__ == '__main__':
    main()