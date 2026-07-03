// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from interfaces:msg/FeatureObservations.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__BUILDER_HPP_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "interfaces/msg/detail/feature_observations__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace interfaces
{

namespace msg
{

namespace builder
{

class Init_FeatureObservations_observations
{
public:
  explicit Init_FeatureObservations_observations(::interfaces::msg::FeatureObservations & msg)
  : msg_(msg)
  {}
  ::interfaces::msg::FeatureObservations observations(::interfaces::msg::FeatureObservations::_observations_type arg)
  {
    msg_.observations = std::move(arg);
    return std::move(msg_);
  }

private:
  ::interfaces::msg::FeatureObservations msg_;
};

class Init_FeatureObservations_size
{
public:
  explicit Init_FeatureObservations_size(::interfaces::msg::FeatureObservations & msg)
  : msg_(msg)
  {}
  Init_FeatureObservations_observations size(::interfaces::msg::FeatureObservations::_size_type arg)
  {
    msg_.size = std::move(arg);
    return Init_FeatureObservations_observations(msg_);
  }

private:
  ::interfaces::msg::FeatureObservations msg_;
};

class Init_FeatureObservations_header
{
public:
  Init_FeatureObservations_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_FeatureObservations_size header(::interfaces::msg::FeatureObservations::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_FeatureObservations_size(msg_);
  }

private:
  ::interfaces::msg::FeatureObservations msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::interfaces::msg::FeatureObservations>()
{
  return interfaces::msg::builder::Init_FeatureObservations_header();
}

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__BUILDER_HPP_
