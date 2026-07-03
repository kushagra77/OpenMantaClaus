// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from interfaces:msg/FeatureObservations.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "interfaces/msg/detail/feature_observations__rosidl_typesupport_introspection_c.h"
#include "interfaces/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "interfaces/msg/detail/feature_observations__functions.h"
#include "interfaces/msg/detail/feature_observations__struct.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/header.h"
// Member `header`
#include "std_msgs/msg/detail/header__rosidl_typesupport_introspection_c.h"
// Member `observations`
#include "interfaces/msg/feature_observation.h"
// Member `observations`
#include "interfaces/msg/detail/feature_observation__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  interfaces__msg__FeatureObservations__init(message_memory);
}

void interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_fini_function(void * message_memory)
{
  interfaces__msg__FeatureObservations__fini(message_memory);
}

size_t interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__size_function__FeatureObservations__observations(
  const void * untyped_member)
{
  const interfaces__msg__FeatureObservation__Sequence * member =
    (const interfaces__msg__FeatureObservation__Sequence *)(untyped_member);
  return member->size;
}

const void * interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_const_function__FeatureObservations__observations(
  const void * untyped_member, size_t index)
{
  const interfaces__msg__FeatureObservation__Sequence * member =
    (const interfaces__msg__FeatureObservation__Sequence *)(untyped_member);
  return &member->data[index];
}

void * interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_function__FeatureObservations__observations(
  void * untyped_member, size_t index)
{
  interfaces__msg__FeatureObservation__Sequence * member =
    (interfaces__msg__FeatureObservation__Sequence *)(untyped_member);
  return &member->data[index];
}

void interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__fetch_function__FeatureObservations__observations(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const interfaces__msg__FeatureObservation * item =
    ((const interfaces__msg__FeatureObservation *)
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_const_function__FeatureObservations__observations(untyped_member, index));
  interfaces__msg__FeatureObservation * value =
    (interfaces__msg__FeatureObservation *)(untyped_value);
  *value = *item;
}

void interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__assign_function__FeatureObservations__observations(
  void * untyped_member, size_t index, const void * untyped_value)
{
  interfaces__msg__FeatureObservation * item =
    ((interfaces__msg__FeatureObservation *)
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_function__FeatureObservations__observations(untyped_member, index));
  const interfaces__msg__FeatureObservation * value =
    (const interfaces__msg__FeatureObservation *)(untyped_value);
  *item = *value;
}

bool interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__resize_function__FeatureObservations__observations(
  void * untyped_member, size_t size)
{
  interfaces__msg__FeatureObservation__Sequence * member =
    (interfaces__msg__FeatureObservation__Sequence *)(untyped_member);
  interfaces__msg__FeatureObservation__Sequence__fini(member);
  return interfaces__msg__FeatureObservation__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_member_array[3] = {
  {
    "header",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(interfaces__msg__FeatureObservations, header),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "size",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(interfaces__msg__FeatureObservations, size),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "observations",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(interfaces__msg__FeatureObservations, observations),  // bytes offset in struct
    NULL,  // default value
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__size_function__FeatureObservations__observations,  // size() function pointer
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_const_function__FeatureObservations__observations,  // get_const(index) function pointer
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__get_function__FeatureObservations__observations,  // get(index) function pointer
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__fetch_function__FeatureObservations__observations,  // fetch(index, &value) function pointer
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__assign_function__FeatureObservations__observations,  // assign(index, value) function pointer
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__resize_function__FeatureObservations__observations  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_members = {
  "interfaces__msg",  // message namespace
  "FeatureObservations",  // message name
  3,  // number of fields
  sizeof(interfaces__msg__FeatureObservations),
  interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_member_array,  // message members
  interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_init_function,  // function to initialize message memory (memory has to be allocated)
  interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_type_support_handle = {
  0,
  &interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_interfaces
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, interfaces, msg, FeatureObservations)() {
  interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, std_msgs, msg, Header)();
  interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, interfaces, msg, FeatureObservation)();
  if (!interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_type_support_handle.typesupport_identifier) {
    interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &interfaces__msg__FeatureObservations__rosidl_typesupport_introspection_c__FeatureObservations_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
