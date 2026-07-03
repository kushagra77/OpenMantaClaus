// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from interfaces:srv/TaskCommand.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__SRV__DETAIL__TASK_COMMAND__TRAITS_HPP_
#define INTERFACES__SRV__DETAIL__TASK_COMMAND__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "interfaces/srv/detail/task_command__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const TaskCommand_Request & msg,
  std::ostream & out)
{
  out << "{";
  // member: command
  {
    out << "command: ";
    rosidl_generator_traits::value_to_yaml(msg.command, out);
    out << ", ";
  }

  // member: initial
  {
    out << "initial: ";
    rosidl_generator_traits::value_to_yaml(msg.initial, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const TaskCommand_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: command
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "command: ";
    rosidl_generator_traits::value_to_yaml(msg.command, out);
    out << "\n";
  }

  // member: initial
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "initial: ";
    rosidl_generator_traits::value_to_yaml(msg.initial, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const TaskCommand_Request & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace interfaces

namespace rosidl_generator_traits
{

[[deprecated("use interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const interfaces::srv::TaskCommand_Request & msg,
  std::ostream & out, size_t indentation = 0)
{
  interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const interfaces::srv::TaskCommand_Request & msg)
{
  return interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<interfaces::srv::TaskCommand_Request>()
{
  return "interfaces::srv::TaskCommand_Request";
}

template<>
inline const char * name<interfaces::srv::TaskCommand_Request>()
{
  return "interfaces/srv/TaskCommand_Request";
}

template<>
struct has_fixed_size<interfaces::srv::TaskCommand_Request>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<interfaces::srv::TaskCommand_Request>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<interfaces::srv::TaskCommand_Request>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace interfaces
{

namespace srv
{

inline void to_flow_style_yaml(
  const TaskCommand_Response & msg,
  std::ostream & out)
{
  out << "{";
  // member: success
  {
    out << "success: ";
    rosidl_generator_traits::value_to_yaml(msg.success, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const TaskCommand_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: success
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "success: ";
    rosidl_generator_traits::value_to_yaml(msg.success, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const TaskCommand_Response & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace srv

}  // namespace interfaces

namespace rosidl_generator_traits
{

[[deprecated("use interfaces::srv::to_block_style_yaml() instead")]]
inline void to_yaml(
  const interfaces::srv::TaskCommand_Response & msg,
  std::ostream & out, size_t indentation = 0)
{
  interfaces::srv::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use interfaces::srv::to_yaml() instead")]]
inline std::string to_yaml(const interfaces::srv::TaskCommand_Response & msg)
{
  return interfaces::srv::to_yaml(msg);
}

template<>
inline const char * data_type<interfaces::srv::TaskCommand_Response>()
{
  return "interfaces::srv::TaskCommand_Response";
}

template<>
inline const char * name<interfaces::srv::TaskCommand_Response>()
{
  return "interfaces/srv/TaskCommand_Response";
}

template<>
struct has_fixed_size<interfaces::srv::TaskCommand_Response>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<interfaces::srv::TaskCommand_Response>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<interfaces::srv::TaskCommand_Response>
  : std::true_type {};

}  // namespace rosidl_generator_traits

namespace rosidl_generator_traits
{

template<>
inline const char * data_type<interfaces::srv::TaskCommand>()
{
  return "interfaces::srv::TaskCommand";
}

template<>
inline const char * name<interfaces::srv::TaskCommand>()
{
  return "interfaces/srv/TaskCommand";
}

template<>
struct has_fixed_size<interfaces::srv::TaskCommand>
  : std::integral_constant<
    bool,
    has_fixed_size<interfaces::srv::TaskCommand_Request>::value &&
    has_fixed_size<interfaces::srv::TaskCommand_Response>::value
  >
{
};

template<>
struct has_bounded_size<interfaces::srv::TaskCommand>
  : std::integral_constant<
    bool,
    has_bounded_size<interfaces::srv::TaskCommand_Request>::value &&
    has_bounded_size<interfaces::srv::TaskCommand_Response>::value
  >
{
};

template<>
struct is_service<interfaces::srv::TaskCommand>
  : std::true_type
{
};

template<>
struct is_service_request<interfaces::srv::TaskCommand_Request>
  : std::true_type
{
};

template<>
struct is_service_response<interfaces::srv::TaskCommand_Response>
  : std::true_type
{
};

}  // namespace rosidl_generator_traits

#endif  // INTERFACES__SRV__DETAIL__TASK_COMMAND__TRAITS_HPP_
