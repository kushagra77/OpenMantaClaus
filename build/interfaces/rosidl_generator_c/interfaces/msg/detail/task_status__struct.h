// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from interfaces:msg/TaskStatus.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_H_
#define INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'task'
// Member 'message'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/TaskStatus in the package interfaces.
typedef struct interfaces__msg__TaskStatus
{
  int32_t state;
  rosidl_runtime_c__String task;
  rosidl_runtime_c__String message;
} interfaces__msg__TaskStatus;

// Struct for a sequence of interfaces__msg__TaskStatus.
typedef struct interfaces__msg__TaskStatus__Sequence
{
  interfaces__msg__TaskStatus * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} interfaces__msg__TaskStatus__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // INTERFACES__MSG__DETAIL__TASK_STATUS__STRUCT_H_
