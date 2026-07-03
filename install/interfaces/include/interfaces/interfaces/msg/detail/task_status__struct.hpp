// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from interfaces:msg/TaskStatus.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_HPP_
#define INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__interfaces__msg__TaskStatus __attribute__((deprecated))
#else
# define DEPRECATED__interfaces__msg__TaskStatus __declspec(deprecated)
#endif

namespace interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct TaskStatus_
{
  using Type = TaskStatus_<ContainerAllocator>;

  explicit TaskStatus_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->state = 0l;
      this->task = "";
      this->message = "";
    }
  }

  explicit TaskStatus_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : task(_alloc),
    message(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->state = 0l;
      this->task = "";
      this->message = "";
    }
  }

  // field types and members
  using _state_type =
    int32_t;
  _state_type state;
  using _task_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _task_type task;
  using _message_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _message_type message;

  // setters for named parameter idiom
  Type & set__state(
    const int32_t & _arg)
  {
    this->state = _arg;
    return *this;
  }
  Type & set__task(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->task = _arg;
    return *this;
  }
  Type & set__message(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->message = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    interfaces::msg::TaskStatus_<ContainerAllocator> *;
  using ConstRawPtr =
    const interfaces::msg::TaskStatus_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<interfaces::msg::TaskStatus_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<interfaces::msg::TaskStatus_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::TaskStatus_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::TaskStatus_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::TaskStatus_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::TaskStatus_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<interfaces::msg::TaskStatus_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<interfaces::msg::TaskStatus_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__interfaces__msg__TaskStatus
    std::shared_ptr<interfaces::msg::TaskStatus_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__interfaces__msg__TaskStatus
    std::shared_ptr<interfaces::msg::TaskStatus_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TaskStatus_ & other) const
  {
    if (this->state != other.state) {
      return false;
    }
    if (this->task != other.task) {
      return false;
    }
    if (this->message != other.message) {
      return false;
    }
    return true;
  }
  bool operator!=(const TaskStatus_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TaskStatus_

// alias to use template instance with default allocator
using TaskStatus =
  interfaces::msg::TaskStatus_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_HPP_
