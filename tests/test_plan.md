# Firmware Verification & Test Plan

> **Honesty Notice**: I do not have physical Arduino hardware on hand, and `arduino-cli` is not installed on this workstation. These test cases are defined and documented as the formal verification specification for running in Wokwi or on hardware. None of these tests have been run or marked as passed yet.

## Test Specification Table

| Test ID | Test Scenario | Input Condition / Action | Expected Result | Status |
| :--- | :--- | :--- | :--- | :--- |
| **TC-01** | Safe Startup State | Boot Arduino Uno with relays wired | Both relays start in open / OFF state (`RELAY_ACTIVE_LOW` safe default) | **NOT RUN** |
| **TC-02** | Dry Soil Threshold (Zone 1) | Set Zone 1 sensor to high ADC ($> 800$, moisture $< 30\%$) | Pump 1 relay turns ON | **NOT RUN** |
| **TC-03** | Dry Soil Threshold (Zone 2) | Set Zone 2 sensor to high ADC ($> 800$, moisture $< 35\%$) | Pump 2 relay turns ON | **NOT RUN** |
| **TC-04** | Hysteresis Deadband Stability | Adjust Zone 1 sensor between $30\%$ and $60\%$ while pump is ON | Relay 1 remains ON; no rapid switching (chatter) | **NOT RUN** |
| **TC-05** | Wet Soil Cutoff | Set Zone 1 sensor to low ADC ($< 550$, moisture $\ge 60\%$) | Pump 1 relay turns OFF | **NOT RUN** |
| **TC-06** | Reverse Hysteresis Retention | Adjust Zone 1 sensor from $60\%$ down to $32\%$ | Relay 1 remains OFF; does not restart until $< 30\%$ | **NOT RUN** |
| **TC-07** | Maximum Continuous Run Time Cutoff | Keep Zone 2 dry ($< 35\%$) continuously for $> 15$ seconds | Pump 2 automatically shuts down; safety lockout engaged | **NOT RUN** |
| **TC-08** | Minimum Rest Time Enforcement | Immediately drop Zone 1 moisture below $30\%$ right after cutoff | Pump 1 must NOT start until 8-second cooldown expires | **NOT RUN** |
| **TC-09** | Disconnected / Floating Sensor | Disconnect A0 signal pin (ADC floating near $1023$) | Out-of-bounds detected ($> 1010$), pump held in safe lockout | **NOT RUN** |
| **TC-10** | Multi-Zone Independence | Zone 1 in wet cutoff while Zone 2 in dry watering | Zone 1 pump OFF, Zone 2 pump ON without cross-interference | **NOT RUN** |

## Instructions for Execution
1. Open the project in Wokwi using [`simulation/diagram.json`](../simulation/diagram.json) and [`firmware/irrigation_system/irrigation_system.ino`](../firmware/irrigation_system/irrigation_system.ino).
2. Connect the Serial Monitor at 9600 baud.
3. For each test case, apply the specified input condition, observe the relay state and serial log, and record the actual result and timestamp here.
