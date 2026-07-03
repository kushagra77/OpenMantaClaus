
#include "tasks/task_runner.hpp"
#include "tasks/task_protocol.hpp"

#include <sstream>

TaskRunner::TaskRunner()
  : rclcpp::Node("task_runner"), tf_buffer_(std::make_shared<tf2_ros::Buffer>(this->get_clock())), tf_listener_(*tf_buffer_) {
  
  active_ = false;
  stop_cooldown_end_time_ = this->now();
  setpoint_updated_ = false;
  frame_count_ = 0;
  params_ = load_task_runner_params(*this);
  curtask_ = params_.default_task;
  debug_flag_ = params_.debug_flag;
  Task::feature_seen.clear();
  for (const auto & feature_name : params_.feature_names) {
    Task::feature_seen[feature_name] = false;
  }

  control_timer_ = this->create_wall_timer(
    std::chrono::milliseconds(static_cast<int>(1000 / params_.control_frequency_hz)),
    std::bind(&TaskRunner::control_loop, this)
  );

  executor_ = create_task_executor(params_.default_task, tf_buffer_, params_.task_config, params_.flare_config, params_.aruco_config, params_.buckets_config, params_.gripper_config, params_.gate_config, params_.qual_gate_config, this->get_clock()->now(), true);
  feature_obs_sub_ = this->create_subscription<interfaces::msg::FeatureObservations>(
    "/tasks/feature_observations", 10,
    std::bind(&TaskRunner::feature_callback, this, std::placeholders::_1));

  bottom_camera_feature_sub_ = this->create_subscription<std_msgs::msg::Int32>(
    "/bottom_camera/angle", 10,
    std::bind(&TaskRunner::bottom_feature_callback, this, std::placeholders::_1));
    
  task_command_service_ = this->create_service<interfaces::srv::TaskCommand>("/tasks/task_command", std::bind(&TaskRunner::update_task, this, std::placeholders::_1, std::placeholders::_2));
  task_complete_client_ = this->create_client<interfaces::srv::TaskComplete>("/tasks/task_complete");
  rc_override_ = this->create_publisher<mavros_msgs::msg::OverrideRCIn>("/mavros/rc/override", 10);
  task_status_pub_ = this->create_publisher<interfaces::msg::TaskStatus>("/tasks/task_status", 10);
  debug_pub_ = this->create_publisher<std_msgs::msg::String>("/debug", 10);
  if (debug_flag_) 
  fps_timer_ = this->create_wall_timer(
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(params_.fps_log_period_s)),
    std::bind(&TaskRunner::log_fps, this));
}

void TaskRunner::publish_debug(const std::string & event, const std::string & detail) {
  if (!debug_flag_ || !debug_pub_) {
    return;
  }

  std_msgs::msg::String msg;
  msg.data = "source=tasks/task_runner event=" + event + " task=" + curtask_ + " " + detail;
  debug_pub_->publish(msg);
}

void TaskRunner::feature_callback(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  for (const auto & obs : msg->observations) {
    Task::feature_seen[params_.feature_names[static_cast<size_t>(obs.id)]] = true;
    if (obs.color != 0) {
      Task::process_colored_feature(obs.id, obs.color);
    }
  }

  if (!active_) return;

  run_executor(msg);
  setpoint_updated_ = true;
  
}

void TaskRunner::bottom_feature_callback(const std_msgs::msg::Int32::SharedPtr msg) {
  if (curtask_ == "gripper") {
    interfaces::msg::FeatureObservation obs;
    obs.bearing = msg->data;
    auto obs_msg = std::make_shared<interfaces::msg::FeatureObservations>();
    obs_msg->observations.push_back(obs);
    obs_msg->header.stamp = static_cast<builtin_interfaces::msg::Time>(this->get_clock()->now());
    run_executor(obs_msg);
    setpoint_updated_ = true;
    publish_debug("Received bottom camera angle", "angle=" + std::to_string(msg->data));
  } else {
    publish_debug("Received bottom camera angle (ignored)", "angle=" + std::to_string(msg->data));
  }
}

void TaskRunner::run_executor(const interfaces::msg::FeatureObservations::SharedPtr msg) {
  if (!active_) return;
  int state_result = executor_->execute(msg);
  
  if (state_result == task_protocol::kTaskComplete) {
    active_ = false;
    cur_state_ = 0;

    if (curtask_ == "gripper") {
      if (curtask_initial_) {
        rc_override(0.0,0.0,1);
        this->get_clock()->sleep_for(rclcpp::Duration::from_seconds(params_.servo_sleep));
        stop();
      } else {
        rc_override(0.0,0.0,2);
        this->get_clock()->sleep_for(rclcpp::Duration::from_seconds(params_.servo_sleep));
        rc_override(params_.gripper_linear_speed / params_.default_speed,0.0,2);
        this->get_clock()->sleep_for(rclcpp::Duration::from_seconds(params_.pickup_sleep));
        stop();
      }
    }

    interfaces::srv::TaskComplete::Request request;
    request.task_completed = true;
    (void)task_complete_client_->async_send_request(std::make_shared<interfaces::srv::TaskComplete::Request>(request));
  } else if (state_result == task_protocol::kTransformUnavailable) {
    stop();
  } else if (state_result != cur_state_){
    auto task_status_msg = interfaces::msg::TaskStatus();
    task_status_msg.task = curtask_;
    task_status_msg.state = state_result;
    task_status_msg.message = "Task in progress";
    task_status_pub_->publish(task_status_msg);
    cur_state_ = state_result;
  }
}

void TaskRunner::update_task(const std::shared_ptr<interfaces::srv::TaskCommand::Request> request,
                             std::shared_ptr<interfaces::srv::TaskCommand::Response> response) {
  curtask_ = request->command;
  curtask_initial_ = request->initial;
  response->success = true;
  active_ = true;
  cur_state_ = 0;

  if (curtask_ == "none") {
    active_ = false;
    response->success = true;
  } else {
    executor_ = create_task_executor(curtask_, tf_buffer_, params_.task_config, params_.flare_config, params_.aruco_config, params_.buckets_config, params_.gripper_config, params_.gate_config, params_.qual_gate_config, this->get_clock()->now(), request->initial);
    if (!executor_) {
      RCLCPP_WARN(this->get_logger(), "Unknown task command: %s", curtask_.c_str());
      response->success = false;
      active_ = false;
    }
  }
}

void TaskRunner::control_loop() {
  if (!active_) {
    stop();
    turnPDReset();
    return; 
  }
  frame_count_ += debug_flag_;

  if (!setpoint_updated_ && curtask_ != "gripper") {
    interfaces::msg::FeatureObservations::SharedPtr empty_msg = std::make_shared<interfaces::msg::FeatureObservations>();
    empty_msg->header.stamp = this->get_clock()->now();
    run_executor(empty_msg);
  }

  setpoint_updated_ = false;
  Task::Pos command = executor_->getCommand();

  if (curtask_ == "gripper") {
    rc_override(command.x*params_.gripper_linear_speed  / params_.default_speed, command.yaw*params_.gripper_yaw_speed);
    return;
  }

  if (command.x == 0.0 && command.yaw == 0.0) {
    stop();
    turnPDReset();
    return;
  } else if (command.x == 0.0 && command.y == 0.0) {
    command.x = task_protocol::kCommandEpsilon;
    rc_override(0.0, turnPD(command.yaw));
    return;
  }
  rc_override(abs(command.yaw) > params_.aggressive_turn_angle_rad ?
              (abs(command.yaw) > 2*params_.aggressive_turn_angle_rad ? 0.0 : 0.5) : 1.0 , turnPD(command.yaw));
}

void TaskRunner::rc_override(double speed, double yaw, int servo, bool set_z_0) {
  if (this->now() < stop_cooldown_end_time_) {
    return;
  }

  auto msg = mavros_msgs::msg::OverrideRCIn();  
  for (int i = 0; i < 18; i++) {
    msg.channels[i] = IGNORE;
  }
  speed *= params_.default_speed;

  if (yaw != IGNORE) {
    yaw = std::max(std::min(yaw, params_.max_turn_speed), -params_.max_turn_speed);
    if (std::abs(yaw * prev_yaw_cmd_) > 1e-6 && std::abs(yaw - prev_yaw_cmd_) > 0.3) {
      yaw = prev_yaw_cmd_ + (yaw - prev_yaw_cmd_) * (10.0/params_.control_frequency_hz);
    }
    msg.channels[3] = task_protocol::kRcNeutralPwm - task_protocol::kRcCommandScale * yaw;
  }
  
  if (speed != IGNORE) {
    if (std::abs(speed * prev_speed_cmd_) > 1e-6 &&
       std::abs(speed - prev_speed_cmd_) > 0.3) {
      speed = prev_speed_cmd_ + (speed - prev_speed_cmd_) * (10.0/params_.control_frequency_hz);
    }
    msg.channels[4] = speed * task_protocol::kRcCommandScale + task_protocol::kRcNeutralPwm;
  }
  if (servo == 2) {
    msg.channels[6] = task_protocol::kRcNeutralPwm - params_.servo_90_pwm_diff;
  } else if (servo == 1) {
    msg.channels[6] = task_protocol::kRcNeutralPwm - params_.servo_90_pwm_diff / 2;
  } else {
    msg.channels[6] = task_protocol::kRcNeutralPwm;
  }
  if (set_z_0) {
    msg.channels[2] = 0;
  }
  rc_override_->publish(msg);
  if (servo != 0) {
    rc_override_->publish(msg);
    rc_override_->publish(msg);
  }
  prev_yaw_cmd_ = yaw;
  prev_speed_cmd_ = speed;

  if (active_) {
    std::ostringstream oss;
    oss << "rc_speed=" << speed
        << " rc_yaw=" << (-yaw)
        << " servo=" << servo;
    publish_debug("rc_override", oss.str());
  }
}

void TaskRunner::log_fps() {
  std::ostringstream oss;
  oss << "fps=" << (frame_count_ / params_.fps_log_period_s);
  publish_debug("fps", oss.str());
  frame_count_ = 0;
}

double TaskRunner::turnPD(double error) {
  rclcpp::Time current_time = this->get_clock()->now();

  if (!pd_initialized_) {
    prev_time_ = current_time;
    prev_error_yaw_ = error;
    pd_initialized_ = true;
    return params_.kp_turn * error;
  }

  rclcpp::Duration dt_duration = current_time - prev_time_;
  double dt = dt_duration.seconds();
  if (dt == 0.0) dt = 1e-6;

  double error_derivative = (error - prev_error_yaw_) / dt;

  double output = params_.kp_turn * error + params_.kd_turn * error_derivative;

  prev_error_yaw_ = error;
  prev_time_ = current_time;
  
  return output;
}

void TaskRunner::stop() {
  rc_override(0.0, 0.0);
  stop_cooldown_end_time_ = this->now() + rclcpp::Duration::from_seconds(params_.stop_cooldown_s);
  turnPDReset();
}

void TaskRunner::turnPDReset() {
  pd_initialized_ = false;
  prev_error_yaw_ = 0.0;
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TaskRunner>());
  rclcpp::shutdown();
  return 0;
}
