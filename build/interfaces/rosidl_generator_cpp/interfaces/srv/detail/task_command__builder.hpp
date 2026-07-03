// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:srv/TaskCommand.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__SRV__DETAIL__TASK_COMMAND__BUILDER_HPP_
#define INTERFACES__SRV__DETAIL__TASK_COMMAND__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/srv/detail/task_command__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace srv
{

namespace builder
{

class Init_TaskCommand_Request_initial
{
public:
  explicit Init_TaskCommand_Request_initial(::interfaces::srv::TaskCommand_Request & msg)
  : msg_(msg)
  {}
  ::interfaces::srv::TaskCommand_Request initial(::interfaces::srv::TaskCommand_Request::_initial_type arg)
  {
    msg_.initial = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::srv::TaskCommand_Request msg_;
};

class Init_TaskCommand_Request_command
{
public:
  Init_TaskCommand_Request_command()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TaskCommand_Request_initial command(::interfaces::srv::TaskCommand_Request::_command_type arg)
  {
    msg_.command = std::move(arg);
    return Init_TaskCommand_Request_initial(msg_);
  }

private:
  ::interfaces::srv::TaskCommand_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::TaskCommand_Request>()
{
  return interfaces::srv::builder::Init_TaskCommand_Request_command();
}

}  // namespace interfaces


namespace interfaces
{

namespace srv
{

namespace builder
{

class Init_TaskCommand_Response_success
{
public:
  Init_TaskCommand_Response_success()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::interfaces::srv::TaskCommand_Response success(::interfaces::srv::TaskCommand_Response::_success_type arg)
  {
    msg_.success = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::srv::TaskCommand_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::TaskCommand_Response>()
{
  return interfaces::srv::builder::Init_TaskCommand_Response_success();
}

}  // namespace interfaces

#endif  // INTERFACES__SRV__DETAIL__TASK_COMMAND__BUILDER_HPP_
