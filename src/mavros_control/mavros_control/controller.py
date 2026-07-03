#! /usr/bin/env python3

from mavros_msgs.srv import CommandBool, SetMode
from tf_transformations import euler_from_quaternion
from rcl_interfaces.msg import ParameterDescriptor
from mavros_msgs.msg import State, OverrideRCIn
from sensor_msgs.msg import Imu
from geographic_msgs.msg import GeoPoseStamped
from rclpy.node import Node
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup
from rclpy.executors import MultiThreadedExecutor
import rclpy

# Keep your utils import
from .utils import *
import numpy as np
import math
import time
import threading

class Controller(Node):

    def __init__(self, node_name='mavros_controller'):
        super().__init__(node_name)
        self.get_logger().info('Initializing Underwater Controller (Async + Safety)')

        # --- Parameters ---
        self.declare_parameter('max_rc_power', 0.5)
        self.declare_parameter('blind_speed', 0.2)
        self.declare_parameter('yaw_kp', 0.4)
        
        self.blind_speed = self.get_parameter('blind_speed').get_parameter_value().double_value
        self.max_rc_power = self.get_parameter('max_rc_power').get_parameter_value().double_value
        self.yaw_kp = self.get_parameter('yaw_kp').get_parameter_value().double_value

        # --- Member Variables ---
        self.vehicle_state = State()
        self.current_yaw = 0.0 
        self.imu_received = False  # Flag to check if we have orientation data
        
        # Initialize RC override with IGNORE (65535)
        self.rc_override = OverrideRCIn()
        self.rc_override.channels = [65535] * 18 
        
        # --- Callback Groups ---
        self.sensor_cb_group = MutuallyExclusiveCallbackGroup()
        self.service_cb_group = MutuallyExclusiveCallbackGroup()
        
        # --- QOS profiles ---
        STATE_QOS = rclpy.qos.QoSProfile(
            depth=10, 
            durability=rclpy.qos.QoSDurabilityPolicy.TRANSIENT_LOCAL
        )
        SENSOR_QOS = rclpy.qos.qos_profile_sensor_data

        # --- Subscribers ---
        # [CHANGED] Use IMU because local_position requires GPS/DVL lock
        self.create_subscription(
            Imu, 
            'mavros/imu/data', 
            self.imu_callback, 
            SENSOR_QOS, 
            callback_group=self.sensor_cb_group
        )
        
        self.create_subscription(
            State, 'mavros/state', self.vehicle_state_callback, 
            STATE_QOS, callback_group=self.sensor_cb_group)

        # --- Publishers ---
        self.rc_override_pub = self.create_publisher(
            OverrideRCIn, 'mavros/rc/override', STATE_QOS)
        
        self.setpoint_pos_pub = self.create_publisher(
            GeoPoseStamped, 'mavros/setpoint_position/global', SENSOR_QOS)
        
        # --- Clients (Using your utils) ---
        self.arm_client = ArmingServiceClient()
        self.set_mode_client = SetModeServiceClient()

    def imu_callback(self, msg):
        """
        Callback for IMU Data (Orientation).
        Replaces Pose callback which requires GPS/DVL.
        """
        orientation_list = [
            msg.orientation.x, 
            msg.orientation.y, 
            msg.orientation.z, 
            msg.orientation.w
        ]
        (roll, pitch, yaw) = euler_from_quaternion(orientation_list)
        
        # self.get_logger().info(f"Yaw: {math.degrees(yaw):.2f}")
        
        self.current_yaw = yaw
        self.imu_received = True

    def vehicle_state_callback(self, msg):
        self.vehicle_state = msg

    def set_target_depth(self, depth):
        target_z = -abs(depth) 
        pose_msg = GeoPoseStamped()
        pose_msg.header.stamp = self.get_clock().now().to_msg()
        pose_msg.header.frame_id = "base_link"
        pose_msg.pose.position.latitude = 0.0 
        pose_msg.pose.position.longitude = 0.0
        pose_msg.pose.position.altitude = target_z

        # Publish a few times to ensure receipt
        for _ in range(5):
            self.setpoint_pos_pub.publish(pose_msg)
            time.sleep(0.01)
        self.get_logger().info(f'Set depth target to {target_z}m')

    def normalize_rc(self, x, outmin=1100, outmax=1900):
        # Clamp to max power
        x = max(min(x, self.max_rc_power), -self.max_rc_power)
        val = (x + 1.0) * (outmax - outmin) / 2.0 + outmin
        return int(min(max(val, outmin), outmax))
    def stop(self):
        self.pub_rc(forward=0.0, throttle=0.0, yaw=0.0)
        
    def pub_rc(self, forward=None, throttle=None, yaw=None):
        def get_pwm(val):
            return self.normalize_rc(val) if val is not None else 65535

        # ARDUSUB MAPPING (Standard)
        # Ch 1: Pitch (Index 0)
        # Ch 2: Roll  (Index 1)
        # Ch 3: Throttle (Index 2)
        # Ch 4: Yaw   (Index 3)
        # Ch 5: Forward (Index 4)
        # Ch 6: Lateral (Index 5)
        if yaw is None:
            yaw = 0
            
        self.rc_override.channels[2] = get_pwm(throttle)  # Index 2 = Throttle
        self.rc_override.channels[3] = get_pwm(yaw)       # Index 3 = Yaw
        self.rc_override.channels[4] = get_pwm(forward)   # Index 4 = Forward 

        self.rc_override_pub.publish(self.rc_override)
        self.rc_override.channels = [65535] * 18

    def arm(self, state=True, timeout_sec=10):
        """Arm/Disarm the vehicle"""
        return self.arm_client.arm(state, timeout_sec)
    
    def set_mode(self, mode=True, timeout_sec=10):
        """Set the vehicle mode"""
        return self.set_mode_client.set_mode(mode, timeout_sec)
        
    def disarm(self, timeout_sec=10):
        """Disarm the vehicle"""
        return self.arm(False, timeout_sec)

    def normalize_angle(self, angle):
        return math.atan2(math.sin(angle), math.cos(angle))

    def turn_yaw(self, degrees):
        # [Safety] Don't turn if we don't have valid IMU data yet
        if not self.imu_received:
            self.get_logger().warn("Cannot turn: No IMU data received yet!")
            return

        target_yaw = self.normalize_angle(self.current_yaw + math.radians(degrees))
        self.get_logger().info(f"Turning {degrees} deg...")

        while rclpy.ok():
            current = self.current_yaw
            # self.get_logger().info(f"Current Yaw: {math.degrees(current):.2f} deg")
            error = self.normalize_angle(target_yaw - current)

            if abs(error) < math.radians(2.0):
                self.pub_rc(yaw=0.0) # Stop turn
                break
            
            yaw_cmd = -1 * self.yaw_kp * error
            if abs(yaw_cmd) < 0.2:
                yaw_cmd = 0.2 if yaw_cmd > 0 else -0.2
            # Send Yaw command, leave others as None (IGNORE)
            self.pub_rc(yaw=yaw_cmd)
            time.sleep(0.02)

    def surface(self):
        """
        Safety routine: Switch to Manual, Ascend, Disarm.
        """
        self.get_logger().warn("INITIATING RESURFACE")
        self.stop()
        
        # 1. Switch to MANUAL to override any autopilot depth logic
        self.set_mode("MANUAL")
        
        # 2. Stop and Disarm
        self.pub_rc(throttle=0.5) # Neutral
        time.sleep(1.0)
        self.get_logger().info("Disarming...")
        self.disarm()
        self.get_logger().info("resurface complete.")

    def run_mission(self):
        
        # Connecting
        # self.disarm() # Ensure we start disarmed
        # self.set_mode("MANUAL") # Start in manual for safety
        # return
        self.get_logger().info('Waiting for connection...')
        while not self.vehicle_state.connected:
            self.get_logger().info('trying to connect...')
            time.sleep(0.5)
        
        # Confirm IMU
        self.get_logger().info('Connected. Waiting for IMU data...')
        while not self.imu_received:
            time.sleep(0.5)
            
        # Just in case
        self.stop()
        self.set_target_depth(0.8)
            
        self.get_logger().info('System Ready. Waiting 15s before starting mission...')
        time.sleep(15) 
        
        # change to depth hold
        while not self.set_mode("ALT_HOLD"):
            self.get_logger().error("Failed to set mode to ALT_HOLD")
        self.get_logger().info("Mode set to ALT_HOLD")
        
        # arming
        while not self.arm(True):
            self.get_logger().error("Failed to arm vehicle")
            self.stop()
        self.get_logger().info("Vehicle Armed")
        
        # Set Depth again just in case
        self.set_target_depth(0.8)
        # time to reach depth
        time.sleep(3.0)
        
        for i in range(2):
            # --- STEP 1.1: GO FORWARD ---
            self.get_logger().info(f"Phase {i*2 + 1}.1: Moving Forward")
            start_time = time.time()
            while time.time() - start_time < 5.0:
                self.pub_rc(forward=self.blind_speed) 
                time.sleep(0.1)

            self.stop()
            self.get_logger().info(f"forwarded")
            time.sleep(0.5)

            # --- STEP 1.2: TURN 90 ---
            self.get_logger().info(f"Phase {i*2 + 1}.2: Turning 90")
            self.turn_yaw(-90)
            time.sleep(0.5)
            self.stop()
            
            # --- STEP 2.1: GO FORWARD---
            self.get_logger().info(f"Phase {i*2 + 2}.1: Moving Forward at 0.5 speed")
            start_time = time.time()
            while time.time() - start_time < 7.0:
                self.pub_rc(forward=self.blind_speed * 0.6) 
                time.sleep(0.1)
                
            self.stop()
            self.get_logger().info(f"forwarded again")
            time.sleep(0.5)
            # --- STEP 2.2: TURN 90 ---
            self.get_logger().info(f"Phase {i*2 + 2}.2: Turning 90")
            self.turn_yaw(-90)
            time.sleep(0.5)
            self.stop()
        
        # self.get_logger().info("Completed rectangle. Now turning 180 for photos.")
        # self.turn_yaw(-180)
        # time.sleep(0.5)
        self.stop()
        
        # time.sleep(60.0) # hover for chessboard photos
    
        # --- FINISH ---
        self.stop()
        self.get_logger().info("Mission Complete. resurfacing.")
        self.surface()

def main(args=None):
    rclpy.init(args=args)
    node = Controller()
    
    executor = MultiThreadedExecutor()
    executor.add_node(node)

    mission_thread = threading.Thread(target=node.run_mission, daemon=True)
    mission_thread.start()

    try:
        executor.spin()
    except KeyboardInterrupt:
        # [NEW] Catch Ctrl+C and run safety sequence
        pass
    finally:
        node.surface()
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()