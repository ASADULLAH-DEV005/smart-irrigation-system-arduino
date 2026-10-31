# Engineering Debug Log & Troubleshooting

This document records the exact issues encountered during firmware authoring and Wokwi simulation testing, how each was identified, and the technical fix implemented.

---

## Real Problems Encountered During Development

### Problem 1: Relay Inverted Logic (Active-LOW Coil Driver)
- **Symptom**: In the simulator, the relay module switched ON immediately on startup when soil was saturated, and turned OFF when soil was dry.
- **Root Cause**: Commercial optocoupled relay modules (and the Wokwi relay model) use PNP transistor or optoisolator drivers where pulling the input pin to GND (LOW) conducts current through the coil, whereas setting it to HIGH (5V) turns it off.
- **Fix**: Added a boolean configuration constant `RELAY_ACTIVE_LOW = true` and wrapped pin writing in a dedicated `setRelay(pin, state)` helper function:
  ```cpp
  void setRelay(uint8_t pin, bool turnOn) {
    if (RELAY_ACTIVE_LOW) {
      digitalWrite(pin, turnOn ? LOW : HIGH);
    } else {
      digitalWrite(pin, turnOn ? HIGH : LOW);
    }
  }
  ```

### Problem 2: Relay Rapid Cycling (Chatter) Around Cutoff Point
- **Symptom**: When moisture hovered right at the threshold (e.g. 35%), the relay clicked on and off repeatedly every read cycle.
- **Root Cause**: ADC readings have small noise fluctuations (±1-2 bits). With a single setpoint, any slight perturbation crossed the boundary back and forth.
- **Fix**: Implemented hysteresis band logic with separate start and stop thresholds (`startMoisturePct = 30%`, `stopMoisturePct = 60%`). In between 30% and 60%, the pump preserves its previous operational state.

### Problem 3: Soil Moisture Noise & ADC Multiplexer Jitter
- **Symptom**: Reading pin A0 right after pin A1 occasionally introduced cross-talk where A1's value leaked into A0's reading.
- **Root Cause**: The ATmega328P has a single internal ADC with an analog multiplexer. High source impedance sensors take time to charge the internal sample-and-hold capacitor (approx 14pF).
- **Fix**: Implemented a 5-sample averaging loop with a 200 microsecond settle delay (`delayMicroseconds(200)`) between reads.

### Problem 4: Safety Runaway on Floating or Disconnected Sensor
- **Symptom**: If an analog pin wire was unplugged, the pin floated near 1023 (raw ADC max), making the microcontroller interpret it as 0% moisture and running the pump indefinitely.
- **Root Cause**: Lack of boundary validation and lack of runtime timeout guards.
- **Fix**:
  1. Added an out-of-bounds safety check: any reading $> 1010$ or $< 10$ trips an immediate sensor fault.
  2. Implemented `MAX_PUMP_RUN_TIME = 15000` ms. If the pump runs continuously past 15 seconds, firmware automatically forces the relay open and enters a safety lockout until reset.

---

## Common Hardware Issues to Watch For in Physical Deployments

These are physical issues that do not appear in a pure digital simulator, but must be accounted for if building physical hardware:

1. **Power Supply Voltage Sag / Brownouts**:
   - DC water pumps draw high inrush stall current when starting up (often 3x to 5x rated running current).
   - If powered from the Arduino 5V regulator, this will drop the 5V rail below 4.5V, triggering the ATmega328P Brown-Out Detector (BOD) and causing random MCU resets.
   - *Prevention*: Always power pumps from an external 12V 2A adapter.
2. **Back-EMF Inductive Spikes**:
   - De-energizing the inductive motor coil produces a high-voltage reverse flyback spike.
   - *Prevention*: Solder a 1N4007 or Schottky diode directly across the pump motor terminals in reverse bias.
3. **Resistive Sensor Electrolysis**:
   - Continuous DC current passing through copper soil probes causes rapid galvanic corrosion within 2-4 weeks.
   - *Prevention*: Use capacitive soil moisture probes (Capacitive v1.2), or power the sensor VCC through an Arduino digital output pin that only turns on for 50ms during reading.
