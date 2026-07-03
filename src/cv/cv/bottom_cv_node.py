#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32
from interfaces.msg import FeatureObservation, FeatureObservations
import socket
import time

class OpenMVBridge(Node):
    def __init__(self):
        super().__init__('openmv_udp_bridge')

        # --- Configuration ---
        self.blueos_ip = "192.168.2.2"  
        self.udp_port = 15000            
        
        # --- Socket Setup ---
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(False) 

        # Initial handshake
        try:
            self.sock.sendto(b'\n', (self.blueos_ip, self.udp_port))
        except Exception as e:
            self.get_logger().warn(f"Initial handshake failed: {e}")

        # --- ROS 2 Publishers & Subscribers ---            
        self.angle_publisher = self.create_publisher(Int32, '/bottom_camera/angle', 10)
        
        # Subscriber to receive 0, 1, or 2
        self.command_subscriber = self.create_subscription(Int32, '/bottom_camera/command', self.command_callback, 10)
        
        # Timer will remain None until a task starts
        self.receive_timer = None
        
        self.get_logger().info(f"OpenMV Bridge Active. Targeting UDP Server at {self.blueos_ip}:{self.udp_port}")
        self.get_logger().info("Waiting for task commands on /bottom_camera/command...")


    def command_callback(self, msg):
        command = msg.data
        
        if command == 0:
            self.get_logger().info("Command 0 received: Stopping tasks.")
            self.send_udp(0)
            self.stop_timer()
            
        elif command in [1, 2]:
            self.get_logger().info(f"Command {command} received: Starting task {command}.")
            
            # Stop any existing timer before starting a new one
            self.stop_timer()
            
            # 1. Send 0 to fix exposure
            self.get_logger().info("Sending 0 to fix exposure. Waiting 5 seconds...")
            self.send_udp(0)
            
            # Sleep blocks the ROS node, but since we have no timer running, it's safe here
            time.sleep(5) 
            
            # Flush any UDP packets that accumulated while we were sleeping
            self.flush_udp_buffer()
            
            # 2. Send the actual task command (1 for Task 1, 2 for Task 2)
            task_msg = command # 1 is putdown, 2 is pickup
            self.get_logger().info(f"Exposure set. Sending task command {task_msg} and starting listener.")
            self.send_udp(task_msg)
            
            # 3. Create the loop to listen for incoming angles - 50 hz
            self.receive_timer = self.create_timer(0.02, self.receive_loop)
            
        else:
            self.get_logger().warn(f"Ignored: Unknown command {command}.")


    def send_udp(self, value):
        """Helper function to send integers over UDP"""
        packet = f"{value}\n".encode('utf-8')
        try:
            self.sock.sendto(packet, (self.blueos_ip, self.udp_port))
            self.get_logger().debug(f"Sent to OpenMV: {value}")
        except Exception as e:
            self.get_logger().error(f"Failed to send UDP packet: {e}")


    def stop_timer(self):
        """Safely destroys the receive timer if it exists"""
        if self.receive_timer is not None:
            self.receive_timer.cancel()
            self.destroy_timer(self.receive_timer)
            self.receive_timer = None
            self.get_logger().info("Receive timer destroyed.")


    def flush_udp_buffer(self):
        """Clears out old packets so the new task starts with fresh data"""
        while True:
            try:
                self.sock.recvfrom(1024)
            except BlockingIOError:
                break # Buffer is empty


    def receive_loop(self):
        try:
            data = None
            while True:
                try:
                    data, _ = self.sock.recvfrom(1024)
                except BlockingIOError:
                    # The buffer is now completely empty. Break the loop.
                    break
            if data:
                msg_str = data.decode('utf-8').strip()
                if msg_str: 
                    '''
                    As a consequence of using FeatureObservation with 0 id the flag will be set
                    to seen. This can be changed but it should be okay at this stage. the flag is
                    always treated as seen anyways.
                    '''
                    
                    received_angle = int(msg_str)                    
                    # obs = FeatureObservation()
                    # obs.bearing = received_angle
                    # ros_msg = FeatureObservations()
                    # ros_msg.observations.append(obs)
                    # ros_msg.header.stamp = self.get_clock().now().to_msg()
                    # ros_msg.header.frame_id = "bottom_camera"
                    ros_msg = Int32()
                    ros_msg.data = received_angle
                    # ros_msg.header = self.get_clock().now().to_msg()  # Add timestamp to the message header
                    self.angle_publisher.publish(ros_msg)
                    self.get_logger().info(f"Received from OpenMV: {received_angle}")
                
        except BlockingIOError:
            pass
        except ValueError:
            self.get_logger().debug(f"Ignored non-integer data: {msg_str}")
        except Exception as e:
            self.get_logger().error(f"UDP receive error: {e}")

def main(args=None):
    rclpy.init(args=args)
    bridge_node = OpenMVBridge()
    try:
        rclpy.spin(bridge_node)
    except KeyboardInterrupt:
        pass
    finally:
        bridge_node.sock.close()
        bridge_node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()