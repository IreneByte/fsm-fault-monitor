# FSM Fault Monitor
Sensor fault detection system built on Arduino. Monitors distance, temperature, and light, triggers named fault codes, forces manual reset, and logs every state transition over serial.
<br /> <br />
![Platform](https://img.shields.io/badge/Platform-Arduino-%233186A0?style=flat-square)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-orange?style=flat-square)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-purple?style=flat-square)
![Schematic](https://img.shields.io/badge/Schematic-KiCad-blue?style=flat-square&logo=KiCad)
![Status](https://img.shields.io/badge/Status-Complete-success?style=flat-square)

## Demo

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

## Fault Code Reference
Code | Source | Description | Trigger Condition
--- | --- | --- | ---
F00 | Manual | Test fault | Triggered via button push
F01 | HC-SR04 | Object jam | Distance < 10 cm for more than 2 seconds
F02 | DHT22 | High temperature | Temperature > 30°C
F03 | Photoresistor (LDR) | Blocked light sensor | Light level < 200 (ADC, 0-1023)

## Getting Started

## Serial Log Format

## Design Decisions

## Known Limitations

## Test Plan
