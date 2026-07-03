// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice

#ifndef INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__FUNCTIONS_H_
#define INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "interfaces/msg/rosidl_generator_c__visibility_control.h"

#include "interfaces/msg/detail/feature_observation__struct.h"

/// Initialize msg/FeatureObservation message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * interfaces__msg__FeatureObservation
 * )) before or use
 * interfaces__msg__FeatureObservation__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__init(interfaces__msg__FeatureObservation * msg);

/// Finalize msg/FeatureObservation message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
void
interfaces__msg__FeatureObservation__fini(interfaces__msg__FeatureObservation * msg);

/// Create msg/FeatureObservation message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * interfaces__msg__FeatureObservation__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
interfaces__msg__FeatureObservation *
interfaces__msg__FeatureObservation__create();

/// Destroy msg/FeatureObservation message.
/**
 * It calls
 * interfaces__msg__FeatureObservation__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
void
interfaces__msg__FeatureObservation__destroy(interfaces__msg__FeatureObservation * msg);

/// Check for msg/FeatureObservation message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__are_equal(const interfaces__msg__FeatureObservation * lhs, const interfaces__msg__FeatureObservation * rhs);

/// Copy a msg/FeatureObservation message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__copy(
  const interfaces__msg__FeatureObservation * input,
  interfaces__msg__FeatureObservation * output);

/// Initialize array of msg/FeatureObservation messages.
/**
 * It allocates the memory for the number of elements and calls
 * interfaces__msg__FeatureObservation__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__Sequence__init(interfaces__msg__FeatureObservation__Sequence * array, size_t size);

/// Finalize array of msg/FeatureObservation messages.
/**
 * It calls
 * interfaces__msg__FeatureObservation__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
void
interfaces__msg__FeatureObservation__Sequence__fini(interfaces__msg__FeatureObservation__Sequence * array);

/// Create array of msg/FeatureObservation messages.
/**
 * It allocates the memory for the array and calls
 * interfaces__msg__FeatureObservation__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
interfaces__msg__FeatureObservation__Sequence *
interfaces__msg__FeatureObservation__Sequence__create(size_t size);

/// Destroy array of msg/FeatureObservation messages.
/**
 * It calls
 * interfaces__msg__FeatureObservation__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
void
interfaces__msg__FeatureObservation__Sequence__destroy(interfaces__msg__FeatureObservation__Sequence * array);

/// Check for msg/FeatureObservation message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__Sequence__are_equal(const interfaces__msg__FeatureObservation__Sequence * lhs, const interfaces__msg__FeatureObservation__Sequence * rhs);

/// Copy an array of msg/FeatureObservation messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_interfaces
bool
interfaces__msg__FeatureObservation__Sequence__copy(
  const interfaces__msg__FeatureObservation__Sequence * input,
  interfaces__msg__FeatureObservation__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // INTERFACES__MSG__DETAIL__FEATURE_OBSERVATION__FUNCTIONS_H_
