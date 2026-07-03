// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__BUILDER_HPP_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/msg/detail/feature_observation__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace msg
{

namespace builder
{

class Init_FeatureObservation_color
{
public:
  explicit Init_FeatureObservation_color(::interfaces::msg::FeatureObservation & msg)
  : msg_(msg)
  {}
  ::interfaces::msg::FeatureObservation color(::interfaces::msg::FeatureObservation::_color_type arg)
  {
    msg_.color = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::msg::FeatureObservation msg_;
};

class Init_FeatureObservation_confident
{
public:
  explicit Init_FeatureObservation_confident(::interfaces::msg::FeatureObservation & msg)
  : msg_(msg)
  {}
  Init_FeatureObservation_color confident(::interfaces::msg::FeatureObservation::_confident_type arg)
  {
    msg_.confident = std::move(arg);
    return Init_FeatureObservation_color(msg_);
  }

private:
  ::interfaces::msg::FeatureObservation msg_;
};

class Init_FeatureObservation_bearing_cov
{
public:
  explicit Init_FeatureObservation_bearing_cov(::interfaces::msg::FeatureObservation & msg)
  : msg_(msg)
  {}
  Init_FeatureObservation_confident bearing_cov(::interfaces::msg::FeatureObservation::_bearing_cov_type arg)
  {
    msg_.bearing_cov = std::move(arg);
    return Init_FeatureObservation_confident(msg_);
  }

private:
  ::interfaces::msg::FeatureObservation msg_;
};

class Init_FeatureObservation_bearing
{
public:
  explicit Init_FeatureObservation_bearing(::interfaces::msg::FeatureObservation & msg)
  : msg_(msg)
  {}
  Init_FeatureObservation_bearing_cov bearing(::interfaces::msg::FeatureObservation::_bearing_type arg)
  {
    msg_.bearing = std::move(arg);
    return Init_FeatureObservation_bearing_cov(msg_);
  }

private:
  ::interfaces::msg::FeatureObservation msg_;
};

class Init_FeatureObservation_id
{
public:
  Init_FeatureObservation_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_FeatureObservation_bearing id(::interfaces::msg::FeatureObservation::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_FeatureObservation_bearing(msg_);
  }

private:
  ::interfaces::msg::FeatureObservation msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::msg::FeatureObservation>()
{
  return interfaces::msg::builder::Init_FeatureObservation_id();
}

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__BUILDER_HPP_
