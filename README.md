# FSM Fault Monitor
Sensor fault detection system built on Arduino. Monitors distance, temperature, and light,
triggers named fault codes, forces manual reset, and logs every state transition over serial.
<br /> <br />
![Platform](https://img.shields.io/badge/Platform-Arduino-%233186A0?style=flat-square)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-orange?style=flat-square)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-purple?style=flat-square)
![Schematic](https://img.shields.io/badge/Schematic-KiCad-blue?style=flat-square&logo=KiCad)
![Status](https://img.shields.io/badge/Status-Complete-success?style=flat-square)

## Overview

## Features

## System Architecture

## Hardware

## Fault Code Reference
### Fault Code Table
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
