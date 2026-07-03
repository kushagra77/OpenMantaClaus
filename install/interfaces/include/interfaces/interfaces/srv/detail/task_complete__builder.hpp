// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:srv/TaskComplete.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__SRV__DETAIL__TASK_COMPLETE__BUILDER_HPP_
#define INTERFACES__SRV__DETAIL__TASK_COMPLETE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/srv/detail/task_complete__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace srv
{

namespace builder
{

class Init_TaskComplete_Request_task_completed
{
public:
  Init_TaskComplete_Request_task_completed()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::interfaces::srv::TaskComplete_Request task_completed(::interfaces::srv::TaskComplete_Request::_task_completed_type arg)
  {
    msg_.task_completed = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::srv::TaskComplete_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::TaskComplete_Request>()
{
  return interfaces::srv::builder::Init_TaskComplete_Request_task_completed();
}

}  // namespace interfaces


namespace interfaces
{

namespace srv
{


}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::TaskComplete_Response>()
{
  return ::interfaces::srv::TaskComplete_Response(rosidl_runtime_cpp::MessageInitialization::ZERO);
}

}  // namespace interfaces

#endif  // INTERFACES__SRV__DETAIL__TASK_COMPLETE__BUILDER_HPP_
