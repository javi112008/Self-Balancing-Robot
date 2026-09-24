# Self-Balancing Robot

A two-wheel robot built with an Arduino Uno, an MPU6050 IMU, and an L298N motor driver. The firmware combines accelerometer and gyroscope measurements to estimate pitch, then uses PID control to drive corrective wheel motion.

I built this project to explore embedded systems and feedback control through hands-on testing. The work involved integrating sensors and motors, filtering noisy measurements, and tuning the controller’s response.

## DEMO

**Original Prototype (made out of cardboard)**
<img width="700" height="1000" alt="IMG_2371" src="https://github.com/user-attachments/assets/86f7657f-6a68-448f-84a4-8d68fc6ab97c" />

**3D MODEL**
<img width="1170" height="1796" alt="IMG_2342" src="https://github.com/user-attachments/assets/3bb1a5f7-2940-4e69-a7c1-6d0d2a08d032" />

**Before tuning**

https://github.com/user-attachments/assets/f33246b4-3727-45be-8490-7fc7e9e3ba52



**After being fully tuned :)**

https://github.com/user-attachments/assets/b1512971-19ff-4db9-85f9-31aef002ed15




## Hardware

| Component | Purpose |
| --- | --- |
| Arduino Uno R3 | Runs the filtering and control loop |
| MPU6050 / GY-521 | Measures acceleration and angular velocity |
| L298N motor driver | Controls motor direction and speed |
| Two TT 1:48 geared motors | Drive the wheels |

## Working Principle

The firmware calculates pitch from the accelerometer and integrates angular velocity from the gyroscope. A complementary filter combines these estimates, using the accelerometer to correct long-term gyro drift while retaining the gyro’s short-term response.

The PID controller compares the estimated pitch with the target angle and calculates a motor command:

- **Proportional:** responds to the current angle error.
- **Integral:** accumulates persistent error.
- **Derivative:** responds to how quickly the error changes.

Both motors receive the same correction. The firmware limits motor output and uses integral clamping with conditional integration to reduce windup. It stops the motors when the estimated pitch reaches ±30°, a sensor read fails, or the control loop exceeds its timing limit. Restarting requires a `go` command.

## Features

- Complementary filter for pitch estimation
- PID control with adjustable gains and target angle
- Control loop with a 10 ms sampling target
- Nonblocking Serial input
- EEPROM storage with a format marker, checksum, and value validation
- Serial telemetry for measured angle, motor output, target angle, and running state
- Motor-output deadband and latched fault stopping
- Explicit `go` and `stop` commands

## Serial Interface

**115200 baud · Newline line ending**

| Command | Follow-up input | Action |
| --- | --- | --- |
| `pid` | Three space-separated values, such as `49 0.2 0.35` | Validates and saves Kp, Ki, and Kd |
| `angle` | Target angle in degrees, such as `6.9` | Validates and saves the balance point |
| `go` | None | Enables balancing when sensor readings are valid and the estimated pitch is within ±20° |
| `stop` | None | Stops the motors and cancels pending input |

The robot starts stopped. Entering `pid` or `angle` stops the motors while sensor sampling continues. After entering new values, send `go` to resume balancing.

PID values must fall between 0 and 1000. The target angle must fall between −20° and +20°. These are input limits, not recommended tuning ranges.

## Software

Written in **C++** using the **Arduino framework** and **PlatformIO**. The MPU6050 interface uses [I2Cdevlib](https://github.com/jrowberg/i2cdevlib); the firmware implements the complementary filter and PID calculations directly.

| File | Description |
| --- | --- |
| `src/main.cpp` | Sensor processing, PID controller, Serial interface, and motor control |
| `docs/WIRING.md` | Pin assignments and sensor orientation |
| `.github/workflows/build.yml` | Automated PlatformIO build workflow |

## Current Limitations

The controller regulates pitch without wheel encoders or position feedback, so the robot can travel while attempting to balance. PID gains, sensor offsets, and the target angle depend on the mechanical setup.

EEPROM writes occur with the motors stopped. The firmware rejects old settings that lack the new validation format and uses source defaults instead.
