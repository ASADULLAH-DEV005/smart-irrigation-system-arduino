# Soil Moisture Sensor Calibration & Analysis

## Physical vs. Simulated Characteristics

### 1. Resistive vs Capacitive Sensors
- **Resistive Sensors (e.g. YL-69 / HL-69)**: Measure electrical conductivity across two exposed copper PCB traces.
  - In air (0% moisture): Resistance approaches infinity. Voltage on analog pin is near VCC (approx. 850 - 950 ADC count).
  - In water (100% moisture): Ions facilitate conduction. Voltage drops to approx. 300 - 380 ADC count.
  - **Limitation**: DC electrolysis corrodes the copper prongs within weeks of continuous soil immersion.
- **Capacitive Sensors (e.g. Capacitive v1.2 / v2.0)**: Measure dielectric permittivity using high frequency oscillation (555 timer circuitry).
  - Traces are insulated from direct water contact, eliminating electrolysis corrosion.
  - Readings are inversely proportional to moisture, similar to resistive dividers.

### 2. Simulator Calibration (Wokwi Potentiometer Mapping)
In the Wokwi simulation environment, a 10k potentiometer functions as the analog soil moisture model:
- Center tap wired to analog pin A0/A1.
- Rotation toward 5V terminal simulates dry soil (ADC reading 850+).
- Rotation toward GND terminal simulates soaked soil (ADC reading 320 or lower).

## Two-Point Calibration Procedure
To calibrate an actual sensor in physical soil before deployment:
1. **Air / Dry Benchmark**:
   - Suspend sensor in open air at ambient humidity.
   - Record raw ADC reading over 50 samples.
   - Example observed reading: `DRY_RAW = 850`.
2. **Saturation Benchmark**:
   - Immerse probe up to the marked line in a cup of soil saturated with water.
   - Let soil settle for 3 minutes to achieve equilibrium.
   - Record raw ADC reading over 50 samples.
   - Example observed reading: `WET_RAW = 320`.
3. **Linear Inverse Formula**:
   $$\text{Moisture (\%)} = \left(\frac{\text{DRY\_RAW} - \text{Raw}}{\text{DRY\_RAW} - \text{WET\_RAW}}\right) \times 100$$
   Constrained between 0% and 100%.

## Hysteresis Rationale
A single threshold (e.g., turn on if below 40%, turn off if above 40%) creates a rapid switching state (contact chattering) when water drops touch the sensor and cause micro-fluctuations.
By applying a hysteresis deadband:
- **Turn ON Threshold**: $< 30\%$ (soil is truly thirsty)
- **Turn OFF Threshold**: $\ge 60\%$ (soil has received adequate deep soak)
- Between $30\%$ and $60\%$, the relay preserves its previous state, preventing repeated mechanical relay clicks.
