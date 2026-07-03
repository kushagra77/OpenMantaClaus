// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:srv/Setup.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__SRV__DETAIL__SETUP__BUILDER_HPP_
#define INTERFACES__SRV__DETAIL__SETUP__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/srv/detail/setup__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace srv
{

namespace builder
{

class Init_Setup_Request_depth
{
public:
  Init_Setup_Request_depth()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::interfaces::srv::Setup_Request depth(::interfaces::srv::Setup_Request::_depth_type arg)
  {
    msg_.depth = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::srv::Setup_Request msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::Setup_Request>()
{
  return interfaces::srv::builder::Init_Setup_Request_depth();
}

}  // namespace interfaces


namespace interfaces
{

namespace srv
{

namespace builder
{

class Init_Setup_Response_success
{
public:
  Init_Setup_Response_success()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::interfaces::srv::Setup_Response success(::interfaces::srv::Setup_Response::_success_type arg)
  {
    msg_.success = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::srv::Setup_Response msg_;
};

}  // namespace builder

}  // namespace srv

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::srv::Setup_Response>()
{
  return interfaces::srv::builder::Init_Setup_Response_success();
}

}  // namespace interfaces

#endif  // INTERFACES__SRV__DETAIL__SETUP__BUILDER_HPP_
