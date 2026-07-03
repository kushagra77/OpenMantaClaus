from typing import List

import numpy as np
from rclpy.node import Node

def load_vector_param(
    node: Node,
    name: str,
    default: List[float],
    expected_size: int,
    dtype,
) -> np.ndarray:
    values = list(node.get_parameter(name).value)
    if len(values) != expected_size:
        node.get_logger().warn(f"Parameter {name} expected {expected_size} values, using defaults")
        values = default
    return np.array(values, dtype=dtype)


def load_matrix3_param(node: Node, name: str, default: List[float], dtype) -> np.ndarray:
    values = list(node.get_parameter(name).value)
    if len(values) != 9:
        node.get_logger().warn(f"Parameter {name} expected 9 values, using defaults")
        values = default
    return np.array(values, dtype=dtype).reshape((3, 3))
