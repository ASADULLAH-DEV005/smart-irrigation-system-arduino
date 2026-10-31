# Firmware Verification & Test Plan

All tests were performed using the Wokwi ATmega328P simulation environment with dual potentiometer sensor emulation and dual relay actuation indicators.

## Test Summary Table

| Test ID | Test Scenario | Input Condition / Action | Expected Result | Simulator Observed Output | Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Safe Startup State | Boot Arduino Uno with relays wired | Both relays start in open / OFF state | `System initialized in safe state. Starting monitoring.` Relays OFF. | **PASS** |
| **TC-02** | Dry Soil Threshold (Zone 1) | Turn Potentiometer 1 to high ADC ($840$, moisture $2\%$) | Pump 1 turns ON because moisture $< 30\%$ | `[ACTION] Soil dry (2%). Turning ON pump for Zone 1 (Tomatoes)` | **PASS** |
| **TC-03** | Dry Soil Threshold (Zone 2) | Turn Potentiometer 2 to high ADC ($835$, moisture $3\%$) | Pump 2 turns ON because moisture $< 35\%$ | `[ACTION] Soil dry (3%). Turning ON pump for Zone 2 (Herbs)` | **PASS** |
| **TC-04** | Hysteresis Deadband Stability | Adjust Potentiometer 1 from $2\%$ to $45\%$ (between $30\%$ and $60\%$) | Relay 1 remains ON; no chatter or cycling | Relay 1 stays ON continuously; no state transition triggered | **PASS** |
| **TC-05** | Wet Soil Cutoff | Turn Potentiometer 1 to low ADC ($510$, moisture $64\%$) | Pump 1 turns OFF because moisture $\ge 60\%$ | `[ACTION] Target moisture reached (64%). Turning OFF pump for Zone 1` | **PASS** |
| **TC-06** | Reverse Hysteresis Retention | Adjust Potentiometer 1 from $64\%$ down to $43\%$ | Relay 1 remains OFF; does NOT turn ON until $< 30\%$ | Relay 1 stays OFF; state preserved | **PASS** |
| **TC-07** | Maximum Continuous Run Time Cutoff | Keep Zone 2 dry ($3\%$) continuously for $> 15$ seconds | Pump 2 automatically shuts down; safety lockout engaged | `[SAFETY] Max run time exceeded for Zone 2 (Herbs). Pump stopped to prevent flooding.` | **PASS** |
| **TC-08** | Minimum Rest Time Enforcement | Immediately drop Zone 1 moisture to $1\%$ after it turned off | Pump 1 must NOT start until 8-second cooldown expires | Pump 1 remained OFF during cooldown, then turned ON at $t = 23$s once 8s elapsed | **PASS** |
| **TC-09** | Disconnected / Floating Sensor | Disconnect A0 signal pin (simulated ADC $= 1023$) | Safety error triggered, pump prevented from energizing | Out-of-bounds detected ($> 1010$), pump held OFF | **PASS** |
| **TC-10** | Multi-Zone Independence | Zone 1 in wet cutoff while Zone 2 in active dry watering | Zone 1 pump OFF, Zone 2 pump ON; no interference | Zone 1 Relay=OFF, Zone 2 Relay=ON | **PASS** |

## Test Execution Notes
- Tests TC-01 through TC-10 were verified by stepping through the Wokwi simulator timeline and matching output strings against `simulation/simulation_output.txt`.
- Multi-zone independence was confirmed by inspecting the timer variables in the `ZoneState` array, ensuring that each zone maintains its own `pumpStartTime` and `pumpStopTime`.
