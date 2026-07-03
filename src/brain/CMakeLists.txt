cmake_minimum_required(VERSION 3.5)
project(brain)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(geographic_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(interfaces REQUIRED)
find_package(std_srvs REQUIRED)

include_directories(
  include
)

add_executable(brain_node
  src/brain.cpp
  src/utils/brain_params.cpp
)

ament_target_dependencies(brain_node
  rclcpp
  mavros_msgs
  geographic_msgs
  sensor_msgs
  interfaces
  std_srvs
)

target_include_directories(brain_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS
  brain_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)


set(CMAKE_UNITY_BUILD ON)

ament_package()
