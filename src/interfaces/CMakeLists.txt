cmake_minimum_required(VERSION 3.5)
project(interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/FeatureObservation.msg"
  "msg/FeatureObservations.msg"
  "msg/TaskStatus.msg"
  "srv/Setup.srv"
  "srv/TaskCommand.srv"
  "srv/TaskComplete.srv"
  DEPENDENCIES std_msgs
)

# This ensures packages linking against 'interfaces' also get 'std_msgs'
ament_export_dependencies(rosidl_default_runtime std_msgs)

ament_package()