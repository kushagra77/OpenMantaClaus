#! /usr/bin/env python3

from mavros_msgs.srv import SetMode, CommandBool, CommandHome, CommandTOL
from rclpy.node import Node
import rclpy


class ArmingServiceClient(Node):
    """Arm/Disarm the vehicle"""
    def __init__(self):
        super().__init__('arming_service_client')
        self.arm_client = self.create_client(CommandBool, 'mavros/cmd/arming')
        while not self.arm_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Arming service not available, waiting again...')
        self.get_logger().info('Arming service available')
        self.arm_request = CommandBool.Request()

    def arm(self, state=True, timeout_sec=30):
        self.arm_request.value = state
        future = self.arm_client.call_async(self.arm_request)
        rclpy.spin_until_future_complete(self, future, 
                                         timeout_sec=timeout_sec)
        return future.result()
    
class SetModeServiceClient(Node):
    """Set the vehicle mode
    Valid flight modes: MANUAL, STABILIZE, ALT_HOLD
    """
    def __init__(self):
        super().__init__('set_mode_service_client')
        self.set_mode_client = self.create_client(SetMode, 'mavros/set_mode')
        while not self.set_mode_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Set mode service not available, waiting again...')
        self.get_logger().info('Set mode service available')
        self.set_mode_request = SetMode.Request()

    def set_mode(self, mode="GUIDED", timeout_sec=30):
        self.set_mode_request.custom_mode = mode
        future = self.set_mode_client.call_async(self.set_mode_request)
        rclpy.spin_until_future_complete(self, future, 
                                         timeout_sec=timeout_sec)
        return future.result()