# Pin Mapping and Wiring Reference

This table lists all physical pin connections for the single-node setup using an Arduino Uno Rev3 and 2-channel relay module.

| Module | Pin on Module | Arduino Uno Pin | Wire Color (Typical) | Purpose / Description |
| :--- | :--- | :--- | :--- | :--- |
| **Zone 1 Moisture Sensor** | VCC | 5V | Red | 5V DC Sensor Power |
| **Zone 1 Moisture Sensor** | GND | GND | Black | Ground Reference |
| **Zone 1 Moisture Sensor** | SIG (Analog Out) | A0 | Green | Zone 1 Soil ADC Reading (0-1023) |
| **Zone 2 Moisture Sensor** | VCC | 5V (Breadboard Rail) | Red | 5V DC Sensor Power |
| **Zone 2 Moisture Sensor** | GND | GND (Breadboard Rail) | Black | Ground Reference |
| **Zone 2 Moisture Sensor** | SIG (Analog Out) | A1 | Yellow | Zone 2 Soil ADC Reading (0-1023) |
| **2-Channel Relay Board** | VCC | 5V | Red | Optocoupler 5V Logic Power |
| **2-Channel Relay Board** | GND | GND | Black | Logic Ground Reference |
| **2-Channel Relay Board** | IN1 | Digital Pin 7 | Orange | Control Signal for Zone 1 (Active-LOW) |
| **2-Channel Relay Board** | IN2 | Digital Pin 8 | Purple | Control Signal for Zone 2 (Active-LOW) |
| **Relay Channel 1 Switch** | COM | +12V External PSU | Red (Heavy gauge) | 12V High-current supply rail |
| **Relay Channel 1 Switch** | NO | Pump 1 (+) Lead | Red | Switched power line to Pump 1 |
| **Relay Channel 2 Switch** | COM | +12V External PSU | Red (Heavy gauge) | 12V High-current supply rail |
| **Relay Channel 2 Switch** | NO | Pump 2 (+) Lead | Red | Switched power line to Pump 2 |
| **Pump 1 Return** | (-) Lead | External PSU GND | Black (Heavy gauge) | Pump 1 ground return |
| **Pump 2 Return** | (-) Lead | External PSU GND | Black (Heavy gauge) | Pump 2 ground return |

## Hardware Safeguard Notes
- **Common Ground**: Do NOT connect the 12V pump negative return to the Arduino GND unless you have verified isolation characteristics. Keeping high-power motor ground separate from sensitive ADC analog ground prevents ground bounce and false moisture readings.
- **Normally Open (NO)**: We explicitly wire to the NO terminal. If the Arduino loses power or resets, the relay de-energizes and the contacts spring open, ensuring water valves and pumps default to OFF.
