# Security & Safety Policy

This document outlines safety-critical thresholds, companion computer constraints, and emergency handling protocols for the OpenMantaClaus autonomous underwater vehicle (AUV).

## Physical & Operational Safety Limits

### 1. Thrust and Speed Clamps
* **Default Speed Scale:** In the task controller parameters (`params.yaml`), the default forward speed is configured at `0.3` (representing approximately 60% of total physical thruster power capacity).
* **Compute-Induced Latency Risk:** During pool trials, raising the speed scale above `0.5` introduced visible response latency. This is due to CPU throttling under visual inference load on the companion computer. For safety, do not command speeds above `0.5` without a high-frequency compute backend (e.g., Nvidia Jetson).

### 2. Surf-on-Failure Design
* The mechanical design of the vehicle is slightly **positively buoyant**. 
* If companion compute or flight controller power fails, the thrusters will shut down, and the vehicle will automatically float to the surface for recovery.

### 3. Emergency Stop (E-Stop) Loop
* The hardware includes a physical E-Stop loop wired through a relay interface. Disconnecting the loop isolates the motor controller power line immediately.
* Refer to the [MantaClaus Electrical & Wiring Schematic](docs/assets/electrical_schematic.jpg) for details.

## Reporting Vulnerabilities

If you discover any security issues, safety-critical bugs, or unstable controller states, please open a Github issue or contact the maintainer directly at `kushagrazaveri@gmail.com`.
