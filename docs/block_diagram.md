# System Architecture & Block Diagram

The automated irrigation system uses a closed-loop sensing and actuation architecture. Analog sensors read soil resistance, an ATmega328P microcontroller filters the signal and calculates moisture levels, and optocoupled relays control low-voltage water pumps.

## Architecture Diagram

```mermaid
flowchart LR
    subgraph Sensing ["Sensing Layer"]
        S1["Zone 1 Moisture Sensor\n(Capacitive / Resistive)"]
        S2["Zone 2 Moisture Sensor\n(Capacitive / Resistive)"]
    end

    subgraph Processing ["Control Unit (Arduino Uno / Nano)"]
        ADC["10-bit ADC\n(Pins A0, A1)"]
        FILTER["Noise Averaging\n(5-sample buffer)"]
        MAP["Moisture Calibration\n(ADC -> 0-100%)"]
        LOGIC["Hysteresis & Safety Engine\n(Zone State Machine)"]
        TIMER["Non-blocking Timers\n(millis clock)"]

        ADC --> FILTER --> MAP --> LOGIC
        TIMER -.-> LOGIC
    end

    subgraph Actuation ["Actuation & Power Isolation"]
        RELAY["2-Channel Relay Module\n(Optocoupler Isolated)"]
        FLY["Flyback Diodes\n(1N4007)"]
        PUMP1["12V DC Submersible Pump\n(Zone 1)"]
        PUMP2["12V DC Submersible Pump\n(Zone 2)"]
        EXT_PWR["External 12V 2A DC Supply"]

        RELAY --> PUMP1
        RELAY --> PUMP2
        FLY --- PUMP1
        FLY --- PUMP2
        EXT_PWR --> RELAY
    end

    S1 -->|Analog Vout| ADC
    S2 -->|Analog Vout| ADC
    LOGIC -->|Digital Out (Active-LOW)| RELAY
```

## Data and Control Flow
1. **Sample Acquisition**: Every 2000 ms, the microcontroller samples analog input channels A0 and A1. Five successive reads are averaged per channel with 200 microseconds between reads to let the internal ADC multiplexer settle.
2. **Moisture Conversion**: Raw 10-bit ADC values (0 to 1023) are mapped against calibration constants:
   - High raw reading = dry soil (high electrical resistance)
   - Low raw reading = saturated soil (low electrical resistance)
3. **Threshold & Hysteresis Evaluation**:
   - If moisture drops below the start threshold (30% for Zone 1, 35% for Zone 2), the controller commands the relay to energize.
   - If moisture rises above the stop threshold (60% for Zone 1, 65% for Zone 2), the controller de-energizes the relay.
   - Values within the deadband do not trigger state switches, preventing rapid contact wear on the relays.
4. **Safety Guards**:
   - Continuous pump run time cannot exceed 15 seconds. If exceeded, the pump is shut off and marked in a safety lockout state.
   - Once a zone shuts off, it must rest for at least 8 seconds before it can turn on again.
