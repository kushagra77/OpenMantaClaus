#include "brain/brain.hpp"

#include <chrono>
#include <cmath>
#include <thread>
#include <future>

using namespace std::chrono_literals;

Brain::Brain(const std::string & name)
: Node(name)
{
  mission_config_ = load_brain_mission_config(*this);
  if (mission_config_.main_run) {
    RCLCPP_INFO(this->get_logger(), "Configured for MAIN ARENA mission sequence.");
  } else {
    RCLCPP_INFO(this->get_logger(), "Configured for QUALIFIER mission sequence.");
  }
  // QoS and subscriptions
  auto qos = rclcpp::QoS(10);
  callback_group_logic_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
  rclcpp::SubscriptionOptions sub_opts;
  sub_opts.callback_group = callback_group_logic_;

  state_sub_ = this->create_subscription<mavros_msgs::msg::State>(
    "mavros/state", qos,
    std::bind(&Brain::state_cb, this, _1));

  setpoint_pub_ = this->create_publisher<geographic_msgs::msg::GeoPoseStamped>("mavros/setpoint_position/global", qos);

  // Create service clients for arming and mode setting
  arm_client_ = this->create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");
  set_mode_client_ = this->create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");

  // Client to activate ekfslam when setup is complete
  ekf_activate_client_ = this->create_client<std_srvs::srv::SetBool>("/ekfslam/activate");

  // choose whether to do qualifier task or full arena
  reset_task_queue();
  
  // more Task stuff
  task_command_client_ = this->create_client<interfaces::srv::TaskCommand>("/cv/task_command");
  task_status_sub_ = this->create_subscription<interfaces::msg::TaskStatus>(
    "/tasks/task_status", qos,
    std::bind(&Brain::task_status_cb, this, _1),
    sub_opts);

  task_complete_service_ = this->create_service<interfaces::srv::TaskComplete>(
    "/tasks/task_complete",
    std::bind(&Brain::task_complete_callback, this, std::placeholders::_1, std::placeholders::_2),
    rmw_qos_profile_services_default,
    callback_group_logic_);

  init_timer_ = this->create_wall_timer(
          std::chrono::milliseconds(0), 
          std::bind(&Brain::run_main_logic, this),
          callback_group_logic_);

  rclcpp::on_shutdown([this]() { this->emergency_shutdown(); });
}

void Brain::reset_task_queue()
{
  task_queue_ = build_task_queue(
    mission_config_.main_run ? mission_config_.main_sequence : mission_config_.qual_sequence);
}


void Brain::run_main_logic() {
  init_timer_->cancel();
  RCLCPP_INFO(this->get_logger(), "Starting main mission logic...");
  perform_setup(
    static_cast<float>(mission_config_.setup_target_depth_m),
    mission_config_.setup_start_delay_s
  );
  
  // Activate EKFSLAM and wait until it reports ready
  RCLCPP_INFO(this->get_logger(), "Activating ekfslam and waiting for ready response...");
  while (rclcpp::ok()) {
    if (!ekf_activate_client_->wait_for_service(std::chrono::seconds(1))) {
      RCLCPP_WARN(this->get_logger(), "Waiting for /ekfslam/activate service...");
      continue;
    }
    auto req = std::make_shared<std_srvs::srv::SetBool::Request>();
    req->data = true;
    auto fut = ekf_activate_client_->async_send_request(req);
    if (fut.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
      auto res = fut.get();
      if (res && res->success) {
        RCLCPP_INFO(this->get_logger(), "ekfslam activated: %s", res->message.c_str());
        break;
      }
      RCLCPP_INFO(this->get_logger(), "ekfslam activation failed: %s", res ? res->message.c_str() : "no response");
    } else {
      RCLCPP_WARN(this->get_logger(), "Timed out waiting for ekfslam activation response, retrying...");
    }
    std::this_thread::sleep_for(500ms);
  }

  RCLCPP_INFO(this->get_logger(), "Setup complete, starting tasks.");
  run_task();
}

void Brain::run_task() {
  if (task_queue_.empty()) {
    RCLCPP_INFO(this->get_logger(), "All tasks completed!");
    interfaces::srv::TaskCommand::Request request;
    request.command = "none";
    (void)task_command_client_->async_send_request(std::make_shared<interfaces::srv::TaskCommand::Request>(request));
    surface();
    return;
  }
  std::string current_task = task_queue_.front();
  interfaces::srv::TaskCommand::Request request;
  if (current_task[0] == '-') {
    request.command = current_task.substr(1); // remove '-' prefix for command
    request.initial = false; // set to false to indicate reverse state
  } else {
    request.command = current_task;
    request.initial = true; 
  }
  task_queue_.pop();
  RCLCPP_INFO(this->get_logger(), "Requesting task: %s", current_task.c_str());
  while (rclcpp::ok() && !task_command_client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_WARN(this->get_logger(), "Task command service /cv/task_command not available, waiting...");
  }
  if (!rclcpp::ok()) {
    RCLCPP_INFO(this->get_logger(), "Interrupted while waiting for /cv/task_command service");
    return;
  }
  (void)task_command_client_->async_send_request(std::make_shared<interfaces::srv::TaskCommand::Request>(request));
}

void Brain::state_cb(const mavros_msgs::msg::State::SharedPtr msg) {
  this->vehicle_state = *msg;
  if (this->vehicle_state.connected) {
    state_sub_.reset(); // Unsubscribe once connected, might change later
  }
}

void Brain::task_status_cb(const interfaces::msg::TaskStatus::SharedPtr msg) {
  RCLCPP_INFO(this->get_logger(), "Received task status update: task=%s, status=%d, message=%s", 
              msg->task.c_str(), msg->state, msg->message.c_str());
  set_target_depth(static_cast<float>(mission_config_.setup_target_depth_m));
  if (msg->task == "qual_gate" && msg->state == 1) { // if qual gate is reversing
    RCLCPP_INFO(this->get_logger(), "Sending command to CV to set qual gate for reversing...");
    run_task();
  }

}

void Brain::task_complete_callback(const std::shared_ptr<interfaces::srv::TaskComplete::Request> request,
                                  std::shared_ptr<interfaces::srv::TaskComplete::Response> response) {
  RCLCPP_INFO(this->get_logger(), "Task complete received: success=%s", 
              request->task_completed ? "true" : "false");
  if (request->task_completed) {
    run_task();
  } else {
    RCLCPP_INFO(this->get_logger(), "Task reported failure, aborting mission.");
    surface();
  }
}

void Brain::surface() {
  RCLCPP_WARN(this->get_logger(), "INITIATING RESURFACE");
  set_target_depth(0.0f); // Publish surface setpoint
  while (rclcpp::ok() && !this->arm(false)) {
    RCLCPP_INFO(this->get_logger(), "Failed to disarm vehicle");
    std::this_thread::sleep_for(10ms);
  }
  while (rclcpp::ok() && !this->set_mode("MANUAL")) {
    RCLCPP_INFO(this->get_logger(), "Failed to set mode to MANUAL");
    std::this_thread::sleep_for(10ms);
  }
  RCLCPP_INFO(this->get_logger(), "resurface complete.");
}

void Brain::emergency_shutdown() {

  // RCLCPP_INFO(this->get_logger(), "INITIATING EMERGENCY SHUTDOWN");
  std::cout<<"INITIATING EMERGENCY SHUTDOWN" << std::endl;
  // Fire DISARM command
  auto arm_req = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
  arm_req->value = false;
  arm_client_->async_send_request(arm_req);

  // Fire MANUAL mode command
  auto mode_req = std::make_shared<mavros_msgs::srv::SetMode::Request>();
  mode_req->custom_mode = "MANUAL";
  set_mode_client_->async_send_request(mode_req);


  // 3. Freeze the teardown for 100ms so the packets actually leave
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Brain::set_target_depth(float depth) {
  auto pose = geographic_msgs::msg::GeoPoseStamped();
  pose.header.stamp = this->now();
  pose.header.frame_id = "base_link";
  pose.pose.position.latitude = 0.0;
  pose.pose.position.longitude = 0.0;
  pose.pose.position.altitude = -std::abs(depth);

  // Safety: publish multiple times to ensure setpoint is latched by MAVROS.
  for (int i = 0; i < mission_config_.setpoint_publish_count; i++) {
    setpoint_pub_->publish(pose);
    std::this_thread::sleep_for(
      std::chrono::milliseconds(mission_config_.setpoint_publish_interval_ms));
  }
  RCLCPP_INFO(this->get_logger(), "Set depth target to %fm", -pose.pose.position.altitude);
}

bool Brain::arm(bool state) {
  while (rclcpp::ok() && !arm_client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_WARN(this->get_logger(), "Arming service not available, waiting...");
  }
  if (!rclcpp::ok()) {
    RCLCPP_INFO(this->get_logger(), "Interrupted while waiting for arming service");
    return false;
  }
  auto req = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
  req->value = state;
  auto result_future = arm_client_->async_send_request(req);
  if (result_future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
    auto res = result_future.get();
    if (res) {
      RCLCPP_INFO(this->get_logger(), "Arming service returned: %s", res->success ? "true" : "false");
      return res->success;
    }
    return false;
  }
  RCLCPP_INFO(this->get_logger(), "Arming service call timed out");
  return false;
}

bool Brain::set_mode(const std::string & mode) {
  while (rclcpp::ok() && !set_mode_client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_WARN(this->get_logger(), "SetMode service not available, waiting...");
  }
  if (!rclcpp::ok()) {
    RCLCPP_INFO(this->get_logger(), "Interrupted while waiting for SetMode service");
    return false;
  }
  auto req = std::make_shared<mavros_msgs::srv::SetMode::Request>();
  req->custom_mode = mode;
  req->base_mode = 0;
  auto result_future = set_mode_client_->async_send_request(req);
  if (result_future.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
    auto res = result_future.get();
    if (res) {
      RCLCPP_INFO(this->get_logger(), "SetMode service returned: %s", res->mode_sent ? "true" : "false");
      return res->mode_sent;
    }
    return false;
  }
  RCLCPP_INFO(this->get_logger(), "SetMode service call timed out");
  return false;
}

bool Brain::perform_setup(float depth, int delay) {
  auto logger = this->get_logger();

  // edge case, surface if depth is 0
  if (depth == 0.0f) {
    RCLCPP_INFO(logger, "Depth 0.0 requested: calling surface()");
    this->surface();
    return true;
  }

  RCLCPP_INFO(logger, "Waiting for connection...");
  while (rclcpp::ok() && !this->vehicle_state.connected) {
    RCLCPP_INFO(logger, "trying to connect...");
    std::this_thread::sleep_for(100ms);
  }
  RCLCPP_INFO(logger, "Connected.");

  while (rclcpp::ok() && !this->set_mode("MANUAL")) {
    RCLCPP_INFO(logger, "Failed to set mode to MANUAL");
    std::this_thread::sleep_for(100ms);
  }
  while (rclcpp::ok() && !this->arm(false)) {
    RCLCPP_INFO(logger, "Failed to disarm vehicle");
    std::this_thread::sleep_for(100ms);
  }
  RCLCPP_INFO(logger, "Vehicle Disarmed and mode switched to manual for setup.");
  
  this->set_target_depth(depth);

  RCLCPP_INFO(logger, "System Ready. Waiting %ds before starting mission...", delay);
  std::this_thread::sleep_for(std::chrono::seconds(delay));

  while (rclcpp::ok() && !this->set_mode("ALT_HOLD")) {
    RCLCPP_INFO(logger, "Failed to set mode to ALT_HOLD");
    std::this_thread::sleep_for(100ms);
  }
  RCLCPP_INFO(logger, "Mode set to ALT_HOLD");

  this->set_target_depth(depth);


  while (rclcpp::ok() && !this->arm(true)) {
    RCLCPP_INFO(logger, "Failed to arm vehicle");
    std::this_thread::sleep_for(100ms);
  }
  RCLCPP_INFO(logger, "Vehicle Armed");

  this->set_target_depth(depth);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  return true;
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<Brain>();

  // Logic can block while callbacks run
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(node);
  executor.spin();

  rclcpp::shutdown();
  return 0;
}