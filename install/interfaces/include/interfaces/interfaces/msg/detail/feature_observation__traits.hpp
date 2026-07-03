// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__TRAITS_HPP_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "interfaces/msg/detail/feature_observation__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace interfaces
{

namespace msg
{

inline void to_flow_style_yaml(
  const FeatureObservation & msg,
  std::ostream & out)
{
  out << "{";
  // member: id
  {
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << ", ";
  }

  // member: bearing
  {
    out << "bearing: ";
    rosidl_generator_traits::value_to_yaml(msg.bearing, out);
    out << ", ";
  }

  // member: bearing_cov
  {
    out << "bearing_cov: ";
    rosidl_generator_traits::value_to_yaml(msg.bearing_cov, out);
    out << ", ";
  }

  // member: confident
  {
    out << "confident: ";
    rosidl_generator_traits::value_to_yaml(msg.confident, out);
    out << ", ";
  }

  // member: color
  {
    out << "color: ";
    rosidl_generator_traits::value_to_yaml(msg.color, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const FeatureObservation & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "id: ";
    rosidl_generator_traits::value_to_yaml(msg.id, out);
    out << "\n";
  }

  // member: bearing
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bearing: ";
    rosidl_generator_traits::value_to_yaml(msg.bearing, out);
    out << "\n";
  }

  // member: bearing_cov
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bearing_cov: ";
    rosidl_generator_traits::value_to_yaml(msg.bearing_cov, out);
    out << "\n";
  }

  // member: confident
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "confident: ";
    rosidl_generator_traits::value_to_yaml(msg.confident, out);
    out << "\n";
  }

  // member: color
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "color: ";
    rosidl_generator_traits::value_to_yaml(msg.color, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const FeatureObservation & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace interfaces

namespace rosidl_generator_traits
{

[[deprecated("use interfaces::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const interfaces::msg::FeatureObservation & msg,
  std::ostream & out, size_t indentation = 0)
{
  interfaces::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use interfaces::msg::to_yaml() instead")]]
inline std::string to_yaml(const interfaces::msg::FeatureObservation & msg)
{
  return interfaces::msg::to_yaml(msg);
}

template<>
inline const char * data_type<interfaces::msg::FeatureObservation>()
{
  return "interfaces::msg::FeatureObservation";
}

template<>
inline const char * name<interfaces::msg::FeatureObservation>()
{
  return "interfaces/msg/FeatureObservation";
}

template<>
struct has_fixed_size<interfaces::msg::FeatureObservation>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<interfaces::msg::FeatureObservation>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<interfaces::msg::FeatureObservation>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__TRAITS_HPP_
