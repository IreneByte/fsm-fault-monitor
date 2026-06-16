# FSM Fault Monitor
Sensor fault detection system built on Arduino. Monitors distance, temperature, and light, triggers named fault codes, forces manual reset, and logs every state transition over serial.
<br /> <br />
![Platform](https://img.shields.io/badge/Platform-Arduino-%233186A0?style=flat-square)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-orange?style=flat-square)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-purple?style=flat-square)
![Schematic](https://img.shields.io/badge/Schematic-KiCad-blue?style=flat-square&logo=KiCad)
![Status](https://img.shields.io/badge/Status-Complete-success?style=flat-square)

## Demo [![YouTube](https://img.shields.io/badge/See%20the%20demo-red?style=flat-square&logo=YouTube)](https://www.youtube.com/watch?v=zTN1_Ybi4fk)
![Physical Build](https://github.com/IreneByte/FSM-Fault-Monitor/blob/main/src/images/collage.png)

## Overview
### What It Does
FSM Fault Monitor is an Arduino-based fault detection system that continuously watches three sensors in real time: distance, temperature, and light. 

When a reading goes out of range, it triggers a named fault, sounds an alarm, and locks the system until a manual reset is performed. Every state transition is logged with a timestamp over serial. A Python script records the serial output in a CSV file.

### Why DFSM Structure?
1. **Undefined states become impossible.** Each state defines exactly which modes of operation of the machine and state transitions are valid. For instance, IDLE cannot jump immediately to FAULT and FAULT cannot jump to running. The DFSM enforces the transitions, not the programmer's memory.
2. **Behaviour can pertain to certain states.** Fault detection only runs in RUNNING and the buzzer only activates in FAULT. Each state owns its logic, so adding or changing behaviour in one state cannot break the behaviour of another.
3. **Scaling to add more states is clean.** Going from one fake fault in v0 to three more real faults in v2 meant adding conditions inside one case block instead of many nested conditionals.

## Features
- Four-state FSM: IDLE, RUNNING, FAULT, RESET_REQUIRED
- Real-time monitoring of three sensors: distance (HC-SR04), temperature (DHT11/DHT22), and light (LDR)
- Named fault codes displayed on LCD (F00, F01, F02, F03)
- RGB LED state indication: Blue, Green, Red, Orange
- Active buzzer alarm on fault
- Manual fault acknowledgement and reset via push button
- Startup self-test for DHT22 and LDR
- Timestamped serial event logging
- Python serial logger with CSV export

## System Architecture
The system has four states: `IDLE`, `RUNNING`, `FAULT`, and `RESET_REQUIRED`. 

Sensor monitoring and fault detection only occur in the `RUNNING` state. A fault causes the system to transition to `FAULT`, activating the buzzer and disabling all sensor reads. 

Pressing the button acknowledges the fault and moves to `RESET_REQUIRED`, where the system can return to `IDLE` only if the operator confirms the issue is resolved and presses the button again to return to `RUNNING`.

There is no way to transition from `IDLE` or `RESET_REQUIRED` directly to `FAULT`.
### State Diagram
<div align="center">
  <img width="100%" src="https://github.com/IreneByte/FSM-Fault-Monitor/blob/main/src/images/fsm-fault-monitor.png" alt="State diagram">
</div>

## Hardware
### Component List
| Component | Part |
|-----------|------|
| Microcontroller | Arduino Uno R3 |
| Display | LCD1602 (16x2) |
| Temperature & Humidity Sensor | DHT22 |
| Ultrasonic Distance Sensor | HC-SR04 |
| Light Sensor | Photoresistor (LDR) |
| LED | RGB LED (Common Cathode) |
| Buzzer | Active Buzzer |
| Input | Push Button x2 |
| Resistors | 220Ω (RGB LED) x3, 1kΩ (LDR) |

### Schematic Diagram
![Schematic Diagram](https://github.com/IreneByte/FSM-Fault-Monitor/blob/main/src/images/schematic_diagram.png)

### Wiring Diagram
![Wiring Diagram](https://github.com/IreneByte/FSM-Fault-Monitor/blob/main/src/images/wiring_diagram.png)

### Pin Mapping
| Pin | Component | Role |
|-----|-----------|------|
| 2 | Buzzer | Output |
| 3 | RGB LED (Red) | PWM Output |
| 4 | DHT22 | Data |
| 5 | RGB LED (Green) | PWM Output |
| 6 | RGB LED (Blue) | PWM Output |
| 8-13 | LCD1602 | Data/Control |
| A0 | Photoresistor | Analog Input |
| A2 | HC-SR04 (Echo) | Input |
| A3 | HC-SR04 (Trig) | Output |
| A4 | Push Button | Input |
| A5 | Fault Test Button | Input |

## Fault Code Reference
Code | Source | Description | Trigger Condition
--- | --- | --- | ---
F00 | Manual | Test fault | Triggered via button push
F01 | HC-SR04 | Object jam | Distance < 10 cm for more than 2 seconds
F02 | DHT22 | High temperature | Temperature > 30°C
F03 | Photoresistor (LDR) | Blocked light sensor | Light level < 200 (ADC, 0-1023)

## Getting Started
### Prerequisites
**Arduino IDE**
- [LiquidCrystal](https://www.arduino.cc/reference/en/libraries/liquidcrystal/)
- [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
- [NewPing](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home)

**Python**
- Python 3.x
- pyserial
- csv (built-in)
- datetime (built-in)

Install pyserial:
```
python -m pip install pyserial
```

### Wiring
Refer to the wiring diagram in `src/diagram/`.

### Uploading
1. Open `FSM-Fault-Monitor.ino` in Arduino IDE
2. Select board: Arduino Uno
3. Select the correct COM port
4. Click Upload

### Running the Python Logger
Ensure the Arduino is connected on COM3 at 9600 baud, then `serial_logger.py` will run automatically. Events are logged over serial in the following format: [millis] STATE: state_name

Example output:
[6055] STATE: RUNNING
[14376] STATE: IDLE
[19904] STATE: RUNNING
[20753] STATE: FAULT
[23636] STATE: RESET REQUIRED

Logs are saved automatically to a CSV file in the main directory. 

## Design Decisions
### Why `millis()` over `delay()`
`delay()` blocks the entire program for its duration. Using `millis()` keeps the main loop running continuously, which is necessary for real-time sensor reading, debouncing, and tracking system runtime.

### Why NewPing over SR04 library
NewPing is the only HC-SR04 library supported in Wokwi. In addition, it offers a cleaner API with built-in timeout handling compared to manual pulse timing with the basic SR04 library.

### SR04 Self-Test Limitation
The HC-SR04 is not included in the startup self-test. `ping_cm()` returns 0 both when the sensor is out of range and when it is disconnected, making the two conditions indistinguishable without additional hardware.

## Known Limitations
- Sensor readings are not instantaneous and vary under real-world conditions
- The 30°C temperature threshold makes F02 difficult to trigger without an external heat source
- HC-SR04 cannot be verified during startup self-test (see Design Decisions)
- Wokwi simulation does not support all physical components exactly:
  - DHT11 (physical) replaced with DHT22 in simulation
  - LDR part differs slightly from physical photoresistor
- Python logger has manual port configuration (hardcoded COM3, 9600 baud)
- Wokwi web version does not support Python serial integration, so VS Code with Wokwi extension must be used for full simulation
