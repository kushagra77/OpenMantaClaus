// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:msg/TaskStatus.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__TASK_STATUS__BUILDER_HPP_
#define INTERFACES__MSG__DETAIL__TASK_STATUS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/msg/detail/task_status__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace msg
{

namespace builder
{

class Init_TaskStatus_message
{
public:
  explicit Init_TaskStatus_message(::interfaces::msg::TaskStatus & msg)
  : msg_(msg)
  {}
  ::interfaces::msg::TaskStatus message(::interfaces::msg::TaskStatus::_message_type arg)
  {
    msg_.message = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::msg::TaskStatus msg_;
};

class Init_TaskStatus_task
{
public:
  explicit Init_TaskStatus_task(::interfaces::msg::TaskStatus & msg)
  : msg_(msg)
  {}
  Init_TaskStatus_message task(::interfaces::msg::TaskStatus::_task_type arg)
  {
    msg_.task = std::move(arg);
    return Init_TaskStatus_message(msg_);
  }

private:
  ::interfaces::msg::TaskStatus msg_;
};

class Init_TaskStatus_state
{
public:
  Init_TaskStatus_state()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_TaskStatus_task state(::interfaces::msg::TaskStatus::_state_type arg)
  {
    msg_.state = std::move(arg);
    return Init_TaskStatus_task(msg_);
  }

private:
  ::interfaces::msg::TaskStatus msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::msg::TaskStatus>()
{
  return interfaces::msg::builder::Init_TaskStatus_state();
}

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__TASK_STATUS__BUILDER_HPP_
