// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from interfaces:msg/FeatureObservations.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_H_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'observations'
#include "interfaces/msg/detail/feature_observation__struct.h"

/// Struct defined in msg/FeatureObservations in the package interfaces.
typedef struct interfaces__msg__FeatureObservations
{
  std_msgs__msg__Header header;
  int32_t size;
  interfaces__msg__FeatureObservation__Sequence observations;
} interfaces__msg__FeatureObservations;

// Struct for a sequence of interfaces__msg__FeatureObservations.
typedef struct interfaces__msg__FeatureObservations__Sequence
{
  interfaces__msg__FeatureObservations * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} interfaces__msg__FeatureObservations__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATIONS__STRUCT_H_
