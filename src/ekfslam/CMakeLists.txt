cmake_minimum_required(VERSION 3.5)
project(ekfslam)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(mavros_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(interfaces REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(std_srvs REQUIRED)

include_directories(
  include
)

add_executable(ekfslam_node
  src/ekfslam.cpp
  src/utils/slam_params.cpp
)

ament_target_dependencies(ekfslam_node
  rclcpp
  geometry_msgs
  std_msgs
  mavros_msgs
  tf2
  tf2_ros
  interfaces
  tf2_geometry_msgs
  std_srvs
)

target_include_directories(ekfslam_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(ekfslam_node
  Eigen3::Eigen
)

add_executable(odometry_node
  src/odometry_node.cpp
  src/utils/odometry.cpp
)

ament_target_dependencies(odometry_node
  rclcpp
  geometry_msgs
  std_msgs
  mavros_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
)

target_include_directories(odometry_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(odometry_node
  Eigen3::Eigen
)

install(TARGETS
  ekfslam_node
  odometry_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)


set(CMAKE_UNITY_BUILD ON)

ament_package()
