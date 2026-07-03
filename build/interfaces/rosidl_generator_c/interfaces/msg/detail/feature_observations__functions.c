// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from interfaces:msg/FeatureObservations.idl
// generated code does not contain a copyright notice
#include "interfaces/msg/detail/feature_observations__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `observations`
#include "interfaces/msg/detail/feature_observation__functions.h"

bool
interfaces__msg__FeatureObservations__init(interfaces__msg__FeatureObservations * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    interfaces__msg__FeatureObservations__fini(msg);
    return false;
  }
  // size
  // observations
  if (!interfaces__msg__FeatureObservation__Sequence__init(&msg->observations, 0)) {
    interfaces__msg__FeatureObservations__fini(msg);
    return false;
  }
  return true;
}

void
interfaces__msg__FeatureObservations__fini(interfaces__msg__FeatureObservations * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // size
  // observations
  interfaces__msg__FeatureObservation__Sequence__fini(&msg->observations);
}

bool
interfaces__msg__FeatureObservations__are_equal(const interfaces__msg__FeatureObservations * lhs, const interfaces__msg__FeatureObservations * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // size
  if (lhs->size != rhs->size) {
    return false;
  }
  // observations
  if (!interfaces__msg__FeatureObservation__Sequence__are_equal(
      &(lhs->observations), &(rhs->observations)))
  {
    return false;
  }
  return true;
}

bool
interfaces__msg__FeatureObservations__copy(
  const interfaces__msg__FeatureObservations * input,
  interfaces__msg__FeatureObservations * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // size
  output->size = input->size;
  // observations
  if (!interfaces__msg__FeatureObservation__Sequence__copy(
      &(input->observations), &(output->observations)))
  {
    return false;
  }
  return true;
}

interfaces__msg__FeatureObservations *
interfaces__msg__FeatureObservations__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  interfaces__msg__FeatureObservations * msg = (interfaces__msg__FeatureObservations *)allocator.allocate(sizeof(interfaces__msg__FeatureObservations), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(interfaces__msg__FeatureObservations));
  bool success = interfaces__msg__FeatureObservations__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
interfaces__msg__FeatureObservations__destroy(interfaces__msg__FeatureObservations * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    interfaces__msg__FeatureObservations__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
interfaces__msg__FeatureObservations__Sequence__init(interfaces__msg__FeatureObservations__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  interfaces__msg__FeatureObservations * data = NULL;

  if (size) {
    data = (interfaces__msg__FeatureObservations *)allocator.zero_allocate(size, sizeof(interfaces__msg__FeatureObservations), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = interfaces__msg__FeatureObservations__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        interfaces__msg__FeatureObservations__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
interfaces__msg__FeatureObservations__Sequence__fini(interfaces__msg__FeatureObservations__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      interfaces__msg__FeatureObservations__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

interfaces__msg__FeatureObservations__Sequence *
interfaces__msg__FeatureObservations__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  interfaces__msg__FeatureObservations__Sequence * array = (interfaces__msg__FeatureObservations__Sequence *)allocator.allocate(sizeof(interfaces__msg__FeatureObservations__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = interfaces__msg__FeatureObservations__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
interfaces__msg__FeatureObservations__Sequence__destroy(interfaces__msg__FeatureObservations__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    interfaces__msg__FeatureObservations__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
interfaces__msg__FeatureObservations__Sequence__are_equal(const interfaces__msg__FeatureObservations__Sequence * lhs, const interfaces__msg__FeatureObservations__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!interfaces__msg__FeatureObservations__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
interfaces__msg__FeatureObservations__Sequence__copy(
  const interfaces__msg__FeatureObservations__Sequence * input,
  interfaces__msg__FeatureObservations__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(interfaces__msg__FeatureObservations);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    interfaces__msg__FeatureObservations * data =
      (interfaces__msg__FeatureObservations *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!interfaces__msg__FeatureObservations__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          interfaces__msg__FeatureObservations__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!interfaces__msg__FeatureObservations__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
