// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_H_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/FeatureObservation in the package interfaces.
typedef struct interfaces__msg__FeatureObservation
{
  int32_t id;
  /// radians in camera/local frame
  double bearing;
  /// variance (rad^2)
  double bearing_cov;
  /// true when identity is confidently assigned
  bool confident;
  /// optional
  uint8_t color;
} interfaces__msg__FeatureObservation;

// Struct for a sequence of interfaces__msg__FeatureObservation.
typedef struct interfaces__msg__FeatureObservation__Sequence
{
  interfaces__msg__FeatureObservation * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} interfaces__msg__FeatureObservation__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__STRUCT_H_
