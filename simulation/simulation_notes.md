# Simulation Setup (Wokwi)

## Overview
Because I do not have physical Arduino hardware on my desk and `arduino-cli` is not installed on this machine, the circuit and firmware were designed to be simulated in [Wokwi](https://wokwi.com).

## Components Configured in `diagram.json`
- **Microcontroller**: Arduino Uno Rev3
- **Sensors**: 2x 10k potentiometers connected to analog pins A0 and A1, functioning as analog dividers to simulate variable soil moisture.
- **Actuators**: 2x single-channel 5V relay modules connected to Digital Pins 7 and 8.
- **Indicators**: 2x blue LEDs with 330 ohm resistors connected across the relay Normally Open (NO) contacts to visually indicate when a pump is running.

## How to Run in Wokwi
1. Open [wokwi.com](https://wokwi.com) in your browser.
2. Select **Arduino Uno**.
3. Copy and paste the code from [`firmware/irrigation_system/irrigation_system.ino`](../firmware/irrigation_system/irrigation_system.ino) into `sketch.ino`.
4. Switch to the `diagram.json` tab and paste the contents of [`simulation/diagram.json`](diagram.json).
5. Click **Start Simulation**.
6. Open the Serial Monitor pane (9600 baud).
7. Turn the potentiometers to simulate dry soil (high voltage / clockwise) and wet soil (low voltage / counter-clockwise), and follow the test cases in [`tests/test_plan.md`](../tests/test_plan.md).
