#pragma once

namespace task_protocol {
constexpr int kTransformUnavailable = -10;
constexpr int kTaskComplete = -1;

constexpr int kRcNeutralPwm = 1500;
constexpr int kRcCommandScale = 500;

constexpr double kCommandEpsilon = 0.0001;
}  // namespace task_protocol
