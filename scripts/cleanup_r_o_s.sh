#!/bin/bash
echo "Cleaning up old ROS processes and DDS memory..."
pkill -9 -f ros
pkill -9 -f ekfslam
pkill -9 -f cv
pkill -9 -f brain
rm -rf /dev/shm/fastrtps_*
ros2 daemon stop
ros2 daemon start
echo "ROS2 cleanup complete."