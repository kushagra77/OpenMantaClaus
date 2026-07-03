
#ifndef TASK_RUNNER_HPP
#define TASK_RUNNER_HPP

#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include "tasks/task.hpp"
#include "tasks/controllers/task_factory.hpp"
#include "tasks/utils/task_runner_params.hpp"

#include "interfaces/srv/task_command.hpp"
#include "interfaces/srv/task_complete.hpp"
#include "interfaces/msg/feature_observations.hpp"
#include "std_msgs/msg/int32.hpp"
#include "interfaces/msg/task_status.hpp"
#include "mavros_msgs/msg/override_rc_in.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::placeholders;

/**
 * @brief TaskRunner node for executing tasks based on feature observations
 */
class TaskRunner : public rclcpp::Node {
public:
  TaskRunner();

private:
  bool active_;
  bool debug_flag_;
  bool setpoint_updated_;
  std::string curtask_;
  bool curtask_initial_;
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  tf2_ros::TransformListener tf_listener_;
  std::unique_ptr<Task> executor_;
  constexpr static int IGNORE = mavros_msgs::msg::OverrideRCIn::CHAN_NOCHANGE; 
  TaskRunnerParams params_;

  int frame_count_;
  rclcpp::Time stop_cooldown_end_time_;
  rclcpp::TimerBase::SharedPtr fps_timer_;
  rclcpp::TimerBase::SharedPtr control_timer_;
  
  rclcpp::Service<interfaces::srv::TaskCommand>::SharedPtr task_command_service_;
  rclcpp::Subscription<interfaces::msg::FeatureObservations>::SharedPtr feature_obs_sub_;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr bottom_camera_feature_sub_;
  rclcpp::Client<interfaces::srv::TaskComplete>::SharedPtr task_complete_client_;
  rclcpp::Publisher<mavros_msgs::msg::OverrideRCIn>::SharedPtr rc_override_;
  rclcpp::Publisher<interfaces::msg::TaskStatus>::SharedPtr task_status_pub_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr debug_pub_;

  void feature_callback(const interfaces::msg::FeatureObservations::SharedPtr msg);
  void bottom_feature_callback(const std_msgs::msg::Int32::SharedPtr msg);
  void run_executor(const interfaces::msg::FeatureObservations::SharedPtr msg);
  
  // PD Controller methods
  double turnPD(double error);
  void turnPDReset();
  
  // PD Controller attributes
  double prev_error_yaw_ = 0.0;
  rclcpp::Time prev_time_;
  bool pd_initialized_ = false;
  int cur_state_ = 0;
  void control_loop();
  void log_fps();
  void rc_override(double speed, double yaw, int servo=0, bool set_z_0=false);
  double prev_speed_cmd_ = 0.0;
  double prev_yaw_cmd_ = 0.0;
  void stop();
  void publish_debug(const std::string & event, const std::string & detail);

  void update_task(const std::shared_ptr<interfaces::srv::TaskCommand::Request> request,
                   std::shared_ptr<interfaces::srv::TaskCommand::Response> response);
};

#endif // TASK_RUNNER_HPP
