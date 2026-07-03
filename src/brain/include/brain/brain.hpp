// Simple header that declares a minimal controller-like class used by the C++ brain node
#ifndef BRAIN__BRAIN_HPP_
#define BRAIN__BRAIN_HPP_

#include <memory>
#include <queue>
#include <string>
#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "mavros_msgs/msg/state.hpp"
#include "mavros_msgs/srv/command_bool.hpp"
#include "mavros_msgs/srv/set_mode.hpp"
#include "geographic_msgs/msg/geo_pose_stamped.hpp"
#include "interfaces/srv/task_command.hpp"
#include "interfaces/srv/task_complete.hpp"
#include "interfaces/msg/task_status.hpp"
#include "brain/utils/brain_params.hpp"
#include "std_srvs/srv/set_bool.hpp"

using std::placeholders::_1;

class Brain : public rclcpp::Node {
public:
  Brain(const std::string & name = "brain_controller");
  // State flags
  mavros_msgs::msg::State vehicle_state;

  /**
   * @brief main loop logic for the brain node
   * 
   */
  void run_main_logic();

  // ------------- Public methods used by the setup logic ---------------
  /**
   * @brief Disarm and surface the vehicle safely, assuming positive buoyancy
   * (Should ideally send a 0 rc command before calling this function)
   */
  void surface();

  /**
   * @brief Perform emergency shutdown procedures, same as surface but without rclcpp::ok checks
   */
  void emergency_shutdown();

  /**
   * @brief Perform setup including connection, depth setting, mode setting, and arming
   * 
   * @param depth Target depth for the vehicle
   * @param delay Delay before starting the mission
   * @return true if setup was successful
   * @return blocks if setup failed
   */
  bool perform_setup(float depth, int delay);
 
  private:
  std::queue<std::string> task_queue_;
  BrainMissionConfig mission_config_;
  // Service clients
  rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr arm_client_;
  rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr set_mode_client_;
  rclcpp::Client<interfaces::srv::TaskCommand>::SharedPtr task_command_client_;
  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr ekf_activate_client_;
  
  // Service servers
  rclcpp::Service<interfaces::srv::TaskComplete>::SharedPtr task_complete_service_;
  // Subscribers / Publishers / Timers
  rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr state_sub_;
  rclcpp::Subscription<interfaces::msg::TaskStatus>::SharedPtr task_status_sub_;
  rclcpp::Publisher<geographic_msgs::msg::GeoPoseStamped>::SharedPtr setpoint_pub_;
  rclcpp::TimerBase::SharedPtr init_timer_;
  
  void run_task();
  // Callbacks
  void state_cb(const mavros_msgs::msg::State::SharedPtr msg);
  void task_complete_callback(const std::shared_ptr<interfaces::srv::TaskComplete::Request> request,
                             std::shared_ptr<interfaces::srv::TaskComplete::Response> response);
  void task_status_cb(const interfaces::msg::TaskStatus::SharedPtr msg);
  rclcpp::CallbackGroup::SharedPtr callback_group_logic_; // callback group for main logic to avoid blocking

  // --------------------------------------------------------------------------
  /**
   * @brief Set the target depth for the vehicle
   * 
   * @param depth Desired depth in meters (function uses absolute value)
   */
  void set_target_depth(float depth);
  void reset_task_queue();


  /**
   * @brief arm/disarm the vehicle
   * 
   * @param state arm/disarm state
   * @return true if successful
   * @return false if unsuccessful
   */
  bool arm(bool state);
  
  /**
   * @brief Set the vehicle mode
   * 
   * @param mode Desired mode as a string
   * @return true if successful
   * @return false if unsuccessful
   */
  bool set_mode(const std::string & mode);
};

#endif  // BRAIN__BRAIN_HPP_
