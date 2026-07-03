#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32
import socket
import time

class OpenMVBridge(Node):
    def __init__(self):
        super().__init__('openmv_udp_bridge')

        # --- Configuration ---
        self.blueos_ip = "192.168.2.2"  
        self.udp_port = 15000            
        
        # --- Socket Setup (The Fix is Here) ---
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(False) 

        # Send a dummy "handshake" byte to register our private port with the BlueOS UDP Server.
        # The OpenMV try/except block will safely ignore this.
        try:
            self.sock.sendto(b'\n', (self.blueos_ip, self.udp_port))
        except Exception as e:
            self.get_logger().warn(f"Initial handshake failed: {e}")

        # --- ROS 2 Publishers & Subscribers ---
        self.subscription = self.create_subscription(
            Int32,
            '/openmv/send_angle',
            self.send_callback,
            10)
            
        self.publisher_ = self.create_publisher(Int32, '/openmv/received_angle', 10)
        self.timer = self.create_timer(0.02, self.receive_loop)
        
        self.get_logger().info(f"OpenMV Bridge Active. Targeting UDP Server at {self.blueos_ip}:{self.udp_port}")
        time.sleep(15)
        self.send_callback(Int32(data=0))
        time.sleep(15)
        self.send_callback(Int32(data=70))
        

    def send_callback(self, msg):
        angle = msg.data
        if 0 <= angle <= 360:
            packet = f"{angle}\n".encode('utf-8')
            try:
                self.sock.sendto(packet, (self.blueos_ip, self.udp_port))
                self.get_logger().debug(f"Sent to OpenMV: {angle}")
            except Exception as e:
                self.get_logger().error(f"Failed to send UDP packet: {e}")
        else:
            self.get_logger().warn(f"Ignored: Angle {angle} is out of bounds.")

    def receive_loop(self):
        try:
            data, addr = self.sock.recvfrom(1024)
            if data:
                msg_str = data.decode('utf-8').strip()
                if msg_str: # Ensure it's not empty
                    received_angle = int(msg_str)                    
                    ros_msg = Int32()
                    ros_msg.data = received_angle
                    self.publisher_.publish(ros_msg)
                    self.get_logger().info(f"Received from OpenMV: {received_angle}")
                
        except BlockingIOError:
            pass
        except ValueError:
            self.get_logger().error(f"Failed to convert received data to integer: {msg_str}")
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
        rclpy.shutdown()

if __name__ == '__main__':
    main()