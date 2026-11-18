# AI_PROJECT.md

## Project Name

BalanceRobot_demo

---

# Project Goal

Build a reusable two-wheel self-balancing robot platform.

This project is intended as a reference implementation for future robotics projects.

Primary goals:

* Stable balancing
* Modular architecture
* Arduino IDE compatibility
* Reusable libraries
* AI-assisted development

---

# Target Hardware

## MCU

ATmega328P

Examples:

* Arduino Uno
* Arduino Nano

---

## IMU

MPU6050

Functions:

* Accelerometer
* Gyroscope
* Angle estimation

---

## Motor Driver

ROHM BD62130AEFJ

Requirements:

* Forward
* Reverse
* PWM speed control

Avoid immediate direction reversal at maximum PWM.

Preferred sequence:

```text
PWM = 0
wait
change direction
apply PWM
```

---

## Motors

Two DC gear motors.

Differential drive configuration.

---

## RGB Indicator

VNEHC-RGB

Used for robot status display.

---

## Communication

Bluetooth Serial

Examples:

* HC05
* HC06
* BLE UART compatible modules

Used for:

* PID tuning
* Runtime configuration
* Diagnostics

---

## Optional Hardware

### Ultrasonic Sensor

Examples:

* HC-SR04

Capabilities:

* Distance measurement
* Obstacle detection
* Emergency stop

---

### Encoder

Optional but recommended.

Capabilities:

* Speed measurement
* Distance estimation
* Cascade PID

---

### I2C Line Sensor

Capabilities:

* Line tracking
* Position estimation
* Steering control

---

# Control Architecture

Preferred architecture:

```text
Bluetooth Command
        │
        ▼
Motion Controller
        │
        ▼
Balance Controller
        │
        ▼
Motor Driver
        │
        ▼
Motors
```

Sensor path:

```text
MPU6050
    │
    ▼
Complementary Filter
    │
    ▼
Balance Controller
```

---

# Angle Estimation

Do not use raw MPU6050 values directly.

Preferred method:

```text
Complementary Filter
```

Default implementation:

```text
98% Gyro
2% Accelerometer
```

unless testing indicates otherwise.

---

# PID Controller

Primary controller:

```text
Angle PID
```

Input:

```text
Target Angle
Current Angle
```

Output:

```text
Motor PWM
```

---

# Anti-Windup

Always implement Anti-Windup.

Preferred methods:

### Saturation Protection

```cpp
if(abs(output) < OUTPUT_LIMIT)
{
    integral += error;
}
```

### Fall Protection

```cpp
if(abs(angle) > FALL_ANGLE)
{
    pid.reset();
}
```

---

# Safety

Robot shall stop balancing when:

```text
abs(angle) > FALL_ANGLE
```

Recommended:

```text
30~45 degrees
```

Actions:

```text
Motor Stop
PID Reset
Status Update
```

---

# Software Modules

Each feature shall be implemented as:

```text
FeatureName.h
FeatureName.cpp
```

Required modules:

```text
BalanceController
MotionController
MotorDriver
ImuFilter
BluetoothCommand
RgbIndicator
```

Optional modules:

```text
UltrasonicSensor
LineSensorI2C
EncoderReader
```

---

# Bluetooth Commands

Support human-readable commands.

Examples:

```text
KP=18.5
KI=0.8
KD=0.15

ANGLE_SP=0

MOVE=FORWARD
MOVE=BACKWARD

TURN=LEFT
TURN=RIGHT

STOP
```

---

# RGB Status Definitions

Suggested states:

```text
BOOTING
CALIBRATING
READY
BALANCING
BLUETOOTH_CONNECTED
OBSTACLE_DETECTED
LINE_TRACKING
FAULT
FALLEN
```

---

# Motion Features

Required:

```text
Stand Still
Forward
Backward
Turn Left
Turn Right
Stop
```

Motion controller should generate:

```text
Target Speed
Target Turn Rate
```

Balance controller remains responsible for keeping the robot upright.

---

# Obstacle Avoidance

If ultrasonic sensor exists:

```text
Distance < Threshold
        │
        ▼
Reduce Speed
        │
        ▼
Stop Robot
```

---

# Line Following

If I2C line sensor exists:

```text
Line Position
        │
        ▼
Steering Command
        │
        ▼
Motion Controller
```

Avoid direct motor manipulation.

---

# Logging

Preferred format:

```text
time,angle,setpoint,output
```

Example:

```text
1000,-1.25,0.00,42
1002,-1.10,0.00,39
```

Avoid excessive Serial output in control loops.

---

# Performance Targets

Balance loop:

```text
500 Hz preferred
```

Acceptable:

```text
200~1000 Hz
```

---

# Code Generation Rules

Before generating new code:

1. Search existing KXN libraries.
2. Reuse existing APIs.
3. Preserve project architecture.
4. Preserve naming conventions.
5. Avoid duplicate implementations.

Do not rewrite stable modules without strong technical justification.

---

# Success Criteria

The project is considered successful when:

* Robot balances reliably.
* Robot can stand still.
* PID can be tuned over Bluetooth.
* RGB status works correctly.
* Motion commands operate correctly.
* Obstacle avoidance functions correctly.
* Line tracking functions correctly.
* Modules are reusable in future projects.
