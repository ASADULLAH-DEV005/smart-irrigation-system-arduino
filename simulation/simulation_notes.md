# Simulation Notes (Wokwi)

## Overview
Because I do not have physical Arduino hardware available, all embedded circuits and firmware logic were simulated and validated using Wokwi (online ATmega328P simulation environment).

## Circuit Setup in Simulator
- **Microcontroller**: Arduino Uno Rev3
- **Soil Sensors**: 2x 10k potentiometers acting as variable analog dividers on pins A0 and A1. Moving the slider changes the analog voltage from 0V to 5V (0 to 1023 ADC).
- **Relay Outputs**: 2x single-channel relay modules connected to Digital Pins 7 and 8.
- **Actuator Simulators**: 2x blue LEDs with 330 ohm current limiting resistors wired across the relay NO (Normally Open) contacts and 5V rail to visually verify relay switching.

## How to Run in Wokwi
1. Open [Wokwi.com](https://wokwi.com) in your browser.
2. Select "Arduino Uno".
3. Replace `sketch.ino` with the contents of `firmware/irrigation_system/irrigation_system.ino`.
4. Switch to the `diagram.json` tab and paste the contents of `simulation/diagram.json`.
5. Click the green "Play" button in the simulator toolbar.
6. Open the Serial Monitor pane at 9600 baud.
7. Adjust the potentiometers to simulate dry soil (high ADC / high resistance) and wet soil (lower ADC / low resistance).
