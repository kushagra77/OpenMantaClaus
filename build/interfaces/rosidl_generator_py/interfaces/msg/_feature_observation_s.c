// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from interfaces:msg/FeatureObservation.idl
// generated code does not contain a copyright notice
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <stdbool.h>
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "numpy/ndarrayobject.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
#include "rosidl_runtime_c/visibility_control.h"
#include "interfaces/msg/detail/feature_observation__struct.h"
#include "interfaces/msg/detail/feature_observation__functions.h"


ROSIDL_GENERATOR_C_EXPORT
bool interfaces__msg__feature_observation__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[55];
    {
      char * class_name = NULL;
      char * module_name = NULL;
      {
        PyObject * class_attr = PyObject_GetAttrString(_pymsg, "__class__");
        if (class_attr) {
          PyObject * name_attr = PyObject_GetAttrString(class_attr, "__name__");
          if (name_attr) {
            class_name = (char *)PyUnicode_1BYTE_DATA(name_attr);
            Py_DECREF(name_attr);
          }
          PyObject * module_attr = PyObject_GetAttrString(class_attr, "__module__");
          if (module_attr) {
            module_name = (char *)PyUnicode_1BYTE_DATA(module_attr);
            Py_DECREF(module_attr);
          }
          Py_DECREF(class_attr);
        }
      }
      if (!class_name || !module_name) {
        return false;
      }
      snprintf(full_classname_dest, sizeof(full_classname_dest), "%s.%s", module_name, class_name);
    }
    assert(strncmp("interfaces.msg._feature_observation.FeatureObservation", full_classname_dest, 54) == 0);
  }
  interfaces__msg__FeatureObservation * ros_message = _ros_message;
  {  // id
    PyObject * field = PyObject_GetAttrString(_pymsg, "id");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->id = (int32_t)PyLong_AsLong(field);
    Py_DECREF(field);
  }
  {  // bearing
    PyObject * field = PyObject_GetAttrString(_pymsg, "bearing");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->bearing = PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // bearing_cov
    PyObject * field = PyObject_GetAttrString(_pymsg, "bearing_cov");
    if (!field) {
      return false;
    }
    assert(PyFloat_Check(field));
    ros_message->bearing_cov = PyFloat_AS_DOUBLE(field);
    Py_DECREF(field);
  }
  {  // confident
    PyObject * field = PyObject_GetAttrString(_pymsg, "confident");
    if (!field) {
      return false;
    }
    assert(PyBool_Check(field));
    ros_message->confident = (Py_True == field);
    Py_DECREF(field);
  }
  {  // color
    PyObject * field = PyObject_GetAttrString(_pymsg, "color");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->color = (uint8_t)PyLong_AsUnsignedLong(field);
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * interfaces__msg__feature_observation__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of FeatureObservation */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("interfaces.msg._feature_observation");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "FeatureObservation");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  interfaces__msg__FeatureObservation * ros_message = (interfaces__msg__FeatureObservation *)raw_ros_message;
  {  // id
    PyObject * field = NULL;
    field = PyLong_FromLong(ros_message->id);
    {
      int rc = PyObject_SetAttrString(_pymessage, "id", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // bearing
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->bearing);
    {
      int rc = PyObject_SetAttrString(_pymessage, "bearing", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // bearing_cov
    PyObject * field = NULL;
    field = PyFloat_FromDouble(ros_message->bearing_cov);
    {
      int rc = PyObject_SetAttrString(_pymessage, "bearing_cov", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // confident
    PyObject * field = NULL;
    field = PyBool_FromLong(ros_message->confident ? 1 : 0);
    {
      int rc = PyObject_SetAttrString(_pymessage, "confident", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }
  {  // color
    PyObject * field = NULL;
    field = PyLong_FromUnsignedLong(ros_message->color);
    {
      int rc = PyObject_SetAttrString(_pymessage, "color", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
