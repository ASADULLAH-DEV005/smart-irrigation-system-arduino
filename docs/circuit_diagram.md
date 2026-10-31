# Circuit Schematic & Connections

## Power Distribution
- **Microcontroller**: Powered via USB (5V) or 9V DC barrel jack.
- **Relay Board Logic**: VCC is supplied by the Arduino 5V rail; GND is shared with Arduino GND.
- **Water Pumps**: Driven by an independent 12V 2A DC wall adapter. **Never power inductive loads like DC pumps directly from Arduino 5V pins**, as current spikes and inductive kickback cause MCU resets and damage voltage regulators.
- **Flyback Protection**: A 1N4007 rectifier diode is connected in reverse bias across the pump terminals (cathode to positive 12V rail, anode to relay switch return) to absorb back-EMF voltage spikes when the relay disconnects.

## Circuit ASCII Diagram

```
                 +5V Rail (Arduino)
                     |
         +-----------+-----------+
         |                       |
       [VCC]                   [VCC]
    +----------+            +----------+
    | Sensor 1 |            | Sensor 2 |
    | (Zone 1) |            | (Zone 2) |
    +----------+            +----------+
       |    |                  |    |
    [SIG]  [GND]            [SIG]  [GND]
      |      |                 |      |
      |   Arduino GND          |   Arduino GND
      |                        |
      +---> Pin A0             +---> Pin A1


    Arduino Uno
    +------------------------------------------+
    | Pin 7 (Digital Out) ---> Relay 1 (IN1)   |
    | Pin 8 (Digital Out) ---> Relay 2 (IN2)   |
    | GND                 ---> Relay GND       |
    | 5V                  ---> Relay VCC       |
    +------------------------------------------+

                   Relay Isolation & Pump Circuit
                   ==============================

       +12V DC Supply ----------------+
                                      |
                                  [Relay COM]
                                      |
                                 \  (Switch)
                                  \
                                      |
                                  [Relay NO]
                                      |
                                      +--------+
                                      |        |
                                    [+]      [CAT]
                                  Pump 1     Diode 1N4007
                                    [-]      [ANO]
                                      |        |
                                      +--------+
                                      |
       12V DC Supply Return (GND) ----+
```

## Isolation Notes
The relay board features PC817 optocouplers. The JD-VCC jumper can be removed to completely isolate the relay coil power supply from the Arduino 5V logic supply, preventing electromagnetic interference (EMI) from coupling back into the ADC lines.
