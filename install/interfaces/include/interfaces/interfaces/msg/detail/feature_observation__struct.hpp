// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_HPP_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__interfaces__msg__FeatureObservation __attribute__((deprecated))
#else
# define DEPRECATED__interfaces__msg__FeatureObservation __declspec(deprecated)
#endif

namespace interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct FeatureObservation_
{
  using Type = FeatureObservation_<ContainerAllocator>;

  explicit FeatureObservation_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->bearing = 0.0;
      this->bearing_cov = 0.0;
      this->confident = false;
      this->color = 0;
    }
  }

  explicit FeatureObservation_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->id = 0l;
      this->bearing = 0.0;
      this->bearing_cov = 0.0;
      this->confident = false;
      this->color = 0;
    }
  }

  // field types and members
  using _id_type =
    int32_t;
  _id_type id;
  using _bearing_type =
    double;
  _bearing_type bearing;
  using _bearing_cov_type =
    double;
  _bearing_cov_type bearing_cov;
  using _confident_type =
    bool;
  _confident_type confident;
  using _color_type =
    uint8_t;
  _color_type color;

  // setters for named parameter idiom
  Type & set__id(
    const int32_t & _arg)
  {
    this->id = _arg;
    return *this;
  }
  Type & set__bearing(
    const double & _arg)
  {
    this->bearing = _arg;
    return *this;
  }
  Type & set__bearing_cov(
    const double & _arg)
  {
    this->bearing_cov = _arg;
    return *this;
  }
  Type & set__confident(
    const bool & _arg)
  {
    this->confident = _arg;
    return *this;
  }
  Type & set__color(
    const uint8_t & _arg)
  {
    this->color = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    interfaces::msg::FeatureObservation_<ContainerAllocator> *;
  using ConstRawPtr =
    const interfaces::msg::FeatureObservation_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::FeatureObservation_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      interfaces::msg::FeatureObservation_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__interfaces__msg__FeatureObservation
    std::shared_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__interfaces__msg__FeatureObservation
    std::shared_ptr<interfaces::msg::FeatureObservation_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const FeatureObservation_ & other) const
  {
    if (this->id != other.id) {
      return false;
    }
    if (this->bearing != other.bearing) {
      return false;
    }
    if (this->bearing_cov != other.bearing_cov) {
      return false;
    }
    if (this->confident != other.confident) {
      return false;
    }
    if (this->color != other.color) {
      return false;
    }
    return true;
  }
  bool operator!=(const FeatureObservation_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct FeatureObservation_

// alias to use template instance with default allocator
using FeatureObservation =
  interfaces::msg::FeatureObservation_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace interfaces

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_HPP_
