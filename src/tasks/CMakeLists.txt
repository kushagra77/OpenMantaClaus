cmake_minimum_required(VERSION 3.5)
project(tasks)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(interfaces REQUIRED)
find_package(mavros REQUIRED)
find_package(mavros_msgs REQUIRED)

add_executable(task_runner
  src/task_runner.cpp
  src/utils/task_runner_params.cpp
  src/controllers/task_factory.cpp
  src/controllers/task.cpp
  src/controllers/qual_gate.cpp
  src/controllers/gate.cpp
  src/controllers/gripper.cpp
  src/controllers/buckets.cpp
  src/controllers/flare.cpp
  src/controllers/aruco.cpp
)

target_link_libraries(task_runner
  tf2_geometry_msgs::tf2_geometry_msgs
)

ament_target_dependencies(task_runner
 rclcpp 
 std_msgs 
 geometry_msgs 
 tf2 
 tf2_ros
 tf2_geometry_msgs
 interfaces 
 mavros 
 mavros_msgs 
)

target_include_directories(task_runner PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS task_runner
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)


set(CMAKE_UNITY_BUILD ON)

ament_package()