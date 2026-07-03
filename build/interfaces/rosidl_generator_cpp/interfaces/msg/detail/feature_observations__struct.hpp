// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from interfaces:msg/FeatureObservations.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_HPP_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"
// Member 'observations'
#include "interfaces/msg/detail/feature_observation__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__interfaces__msg__FeatureObservations __attribute__((deprecated))
#else
# define DEPRECATED__interfaces__msg__FeatureObservations __declspec(deprecated)
#endif

namespace interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct FeatureObservations_
{
  using Type = FeatureObservations_<ContainerAllocator>;

  explicit FeatureObservations_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->size = 0l;
    }
  }

  explicit FeatureObservations_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->size = 0l;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _size_type =
    int32_t;
  _size_type size;
  using _observations_type =
    std::vector<interfaces::msg::FeatureObservation_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<interfaces::msg::FeatureObservation_<ContainerAllocator>>>;
  _observations_type observations;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__size(
    const int32_t & _arg)
  {
    this->size = _arg;
    return *this;
  }
  Type & set__observations(
    const std::vector<interfaces::msg::FeatureObservation_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<interfaces::msg::FeatureObservation_<ContainerAllocator>>> & _arg)
  {
    this->observations = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    interfaces::msg::FeatureObservations_<ContainerAllocator> *;
  using ConstRawPtr =
    const interfaces::msg::FeatureObservations_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::FeatureObservations_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::FeatureObservations_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__interfaces__msg__FeatureObservations
    std::shared_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__interfaces__msg__FeatureObservations
    std::shared_ptr<interfaces::msg::FeatureObservations_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const FeatureObservations_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->size != other.size) {
      return false;
    }
    if (this->observations != other.observations) {
      return false;
    }
    return true;
  }
  bool operator!=(const FeatureObservations_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct FeatureObservations_

// alias to use template instance with default allocator
using FeatureObservations =
  interfaces::msg::FeatureObservations_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_HPP_
