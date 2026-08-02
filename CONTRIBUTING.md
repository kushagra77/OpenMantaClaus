# Contributing to OpenMantaClaus

Thank you for your interest in contributing to OpenMantaClaus! We welcome contributions from other students, researchers, and hobbyists looking to build, test, and improve affordable AUVs.

Here are the guidelines to help you get started:

## Coding Style & Standards

We don't enforce strict linting rules or specific style guides. However, we ask contributors to follow general ROS 2 best practices (e.g., proper node structures, parameter usage, and clear interfaces) to keep the code clean and understandable. The main requirement is that the code compiles successfully and runs correctly on the robot without erratic behavior or performance regressions.

## Development Workflow

1. **Fork the Repository:** Create a personal fork of the repository on GitHub.
2. **Create a Branch:** Create a branch for your bug fix or feature (e.g., `git checkout -b feature/cool-new-improvement`).
3. **Make and Test Changes:**
   * Implement your modifications. Make sure to parameterize any new constants.
   * Verify that your changes build and run correctly.
4. **Submit a Pull Request:** Describe the problem you are solving and your testing procedures.

## Build & Verification

Before submitting a pull request, verify that the workspace builds successfully:

```bash
# Build the workspace
colcon build --symlink-install
source install/setup.bash
```

## Community & Safety

Please remember that this software controls physical robotics hardware. Be mindful of physical safety constraints:
* Avoid modifying safety-critical speed bounds or control limits without extensive simulation and dry testing.
* Document any changes to interface messages or services.

## Potential Contributions / Roadmap

We welcome contributions to help improve the OpenMantaClaus platform! Here are some key areas you can work on:

1. **Visual Inertial Odometry (VIO):** Implementing planar tracking utilizing the bottom-facing camera (e.g., OpenMV) to reduce odometry drift.
2. **Simulation Integration:** Setting up a Gazebo/UUV Simulator environment to enable dry testing of the full autonomous stack.
3. **Parameter Tuning Automation:** Developing automated tuning tools for physical parameters (`thrust_k_f` and `thrust_k_r`).
