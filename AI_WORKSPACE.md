# AI_WORKSPACE.md

## Purpose

This repository is an Arduino-centric robotics workspace.

The workspace is intended to support:

* Arduino AVR
* STM32
* ESP32
* Robotics projects
* Reusable embedded libraries
* AI-assisted development

This document defines workspace-wide rules that apply to all projects.

Project-specific requirements belong in:

```text
projects/<ProjectName>/AI_PROJECT.md
```

---

# Development Environment

Primary development environment:

* Arduino IDE

Optional:

* Arduino CLI
* VSCode
* Codex
* Antigravity
* Claude Code
* Cursor Agent

All generated code must remain compatible with Arduino IDE.

Do not require PlatformIO, CMake, or custom build systems unless explicitly requested.

---

# Arduino IDE Compatibility

Project source files shall follow Arduino IDE conventions.

Preferred structure:

```text
ProjectName/

ProjectName.ino

FeatureA.h
FeatureA.cpp

FeatureB.h
FeatureB.cpp
```

Avoid requiring:

```text
src/
include/
components/
```

unless explicitly requested.

The generated project should compile directly inside Arduino IDE.

---

# Sketch Rules

The .ino file is an application entry point only.

The .ino file should contain:

* includes
* object declarations
* setup()
* loop()

Avoid implementing business logic inside the .ino file.

Bad:

```cpp
void updateMotor()
{
}

void updatePID()
{
}
```

Good:

```cpp
BalanceController balance;

void setup()
{
    balance.begin();
}

void loop()
{
    balance.update();
}
```

Implementation belongs in .cpp files.

---

# Modular Design Policy

Each feature must be implemented as an independent module.

Required format:

```text
FeatureName.h
FeatureName.cpp
```

Examples:

```text
MotorDriver.h
MotorDriver.cpp

BluetoothCommand.h
BluetoothCommand.cpp

RgbIndicator.h
RgbIndicator.cpp
```

Avoid monolithic source files.

Favor reusable modules.

---

# Library Reuse Policy

Before creating new code:

Preferred order:

```text
Reuse existing library
↓
Extend existing library
↓
Create new library
```

Avoid duplicate implementations.

---

# Workspace Library Location

Reusable libraries are stored in:

```text
libraries/
```

Examples:

```text
libraries/

KXN_PID/
KXN_Task/
KXN_MPU6050/
KXN_BD62130/
KXN_RGB/
KXN_Ultrasonic/
```

When suitable libraries already exist, reuse them.

Do not create alternative implementations without justification.

---

# Coding Style

Use:

```cpp
uint8_t
int8_t

uint16_t
int16_t

uint32_t
int32_t
```

Prefer explicit integer types.

Avoid:

```cpp
long
unsigned long
```

when fixed-width types are more appropriate.

---

# Memory Policy

Avoid:

* dynamic allocation
* malloc()
* free()
* new
* delete

unless explicitly required.

Prefer:

* static allocation
* stack allocation
* fixed-size buffers

---

# Real-Time Policy

Prioritize:

1. Reliability
2. Deterministic timing
3. Safety
4. Readability
5. Performance

Avoid unnecessary blocking code.

Avoid excessive Serial printing in control loops.

---

# Naming Convention

Classes:

```cpp
MotorDriver
BalanceController
BluetoothCommand
```

Functions:

```cpp
motorSetPWM()
updateBalance()
readSensor()
```

Variables:

```cpp
currentAngle
targetAngle
motorPwm
```

Constants:

```cpp
MAX_PWM
LOOP_PERIOD_US
```

---

# Documentation Files

The AI assistant should continuously use:

```text
AI_WORKSPACE.md
README.md
TODO.md
CHANGELOG.md
docs/*.md
```

as project context.

Documentation has priority over assumptions.

---

# Project Knowledge Priority

When generating code:

Priority order:

1. AI_WORKSPACE.md
2. AI_PROJECT.md
3. Existing project source code
4. Existing custom libraries
5. Existing documentation
6. Third-party libraries
7. Generic AI knowledge

---

# AI Behavior Rules

Before generating code:

* Inspect existing libraries
* Reuse existing APIs
* Preserve architecture
* Preserve naming conventions

Do not rewrite working modules without technical justification.

Do not introduce new frameworks unnecessarily.

Favor incremental improvements over large refactors.

---

# Workspace Goal

The workspace is intended to support multiple reusable robotics projects.

Examples:

* Self-balancing robot
* Line follower robot
* AGV
* Differential drive robot
* Remote control robot
* Future robotics platforms

Always prioritize:

Reusability
↓
Maintainability
↓
Performance Optimization
