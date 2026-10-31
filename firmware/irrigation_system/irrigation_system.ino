/*
 * Smart Automated Irrigation System - Single Node Controller
 * Author: Abbas Khan
 * Target: Arduino Uno / Nano (ATmega328P)
 *
 * Description:
 * Reads two soil moisture sensors on analog pins, averages raw ADC values to filter noise,
 * maps them to moisture percentages with calibration thresholds, and switches active-low
 * relays with hysteresis and safety timeouts.
 */

// Relay polarity configuration: Most standard 5V optocoupled relay boards
// turn ON when the pin is pulled LOW. Set to true for active-low boards.
const bool RELAY_ACTIVE_LOW = true;

// Timing constants (in milliseconds)
const unsigned long SENSOR_READ_INTERVAL = 2000;   // Read sensors every 2 seconds
const unsigned long LOG_INTERVAL = 3000;          // Output serial telemetry every 3 seconds
const unsigned long MAX_PUMP_RUN_TIME = 15000;    // Safety cutoff: 15s max continuous run in test
const unsigned long MIN_REST_TIME = 8000;         // Cooldown: 8s wait before pump can re-engage

// Number of analog samples to average for noise reduction
const int NUM_SAMPLES = 5;

// Calibration reference points:
// In air / dry soil: high resistance -> high ADC value (around 850-950)
// In cup of water / saturated soil: low resistance -> lower ADC value (around 300-400)
// In Wokwi potentiometers (used as soil sensor simulators): 0 = 0V, 1023 = 5V.
struct ZoneConfig {
  const char* name;
  uint8_t sensorPin;
  uint8_t relayPin;
  int dryRaw;            // ADC value when completely dry
  int wetRaw;            // ADC value when saturated
  int startMoisturePct;  // Turn pump ON if moisture falls below this (%)
  int stopMoisturePct;   // Turn pump OFF if moisture rises above this (%)
};

struct ZoneState {
  int rawReading;
  int moisturePct;
  bool pumpActive;
  bool safetyTripped;
  unsigned long pumpStartTime;
  unsigned long pumpStopTime;
};

// Two independent irrigation zones
const int NUM_ZONES = 2;
ZoneConfig zones[NUM_ZONES] = {
  {
    "Zone 1 (Tomatoes)",
    A0,
    7,
    850,   // Dry calibration
    320,   // Wet calibration
    30,    // Turn ON below 30%
    60     // Turn OFF above 60% (hysteresis band = 30% - 60%)
  },
  {
    "Zone 2 (Herbs)",
    A1,
    8,
    850,
    320,
    35,    // Turn ON below 35%
    65     // Turn OFF above 65%
  }
};

ZoneState zoneStates[NUM_ZONES];

unsigned long lastReadTime = 0;
unsigned long lastLogTime = 0;

// Helper to drive relay based on active-low setting
void setRelay(uint8_t pin, bool turnOn) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(pin, turnOn ? LOW : HIGH);
  } else {
    digitalWrite(pin, turnOn ? HIGH : LOW);
  }
}

// Read analog pin multiple times and take the average to smooth out noise
int averageReading(uint8_t pin, int samples) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200); // short settle time between ADC conversions
  }
  return (int)(sum / samples);
}

// Convert raw ADC reading into an estimated moisture percentage (0 - 100%)
// Soil sensors output high values in dry soil and low values in wet soil
int moistureToPercent(int raw, int dryRaw, int wetRaw) {
  // Constrain to calibrated envelope
  int clamped = constrain(raw, min(wetRaw, dryRaw), max(wetRaw, dryRaw));
  // Invert mapping: dryRaw maps to 0%, wetRaw maps to 100%
  long pct = map(clamped, dryRaw, wetRaw, 0, 100);
  return (int)constrain(pct, 0, 100);
}

// Control decision logic with hysteresis and safety cutoffs
void controlZone(int index, unsigned long currentTime) {
  ZoneConfig &cfg = zones[index];
  ZoneState &st = zoneStates[index];

  // Check for disconnected or damaged sensor (floating wire near max ADC)
  if (st.rawReading > 1010 || st.rawReading < 10) {
    if (st.pumpActive) {
      setRelay(cfg.relayPin, false);
      st.pumpActive = false;
      st.pumpStopTime = currentTime;
    }
    st.safetyTripped = true;
    return;
  }

  // Safety cutoff 1: Has pump been running continuously past maximum allowed time?
  if (st.pumpActive && (currentTime - st.pumpStartTime >= MAX_PUMP_RUN_TIME)) {
    setRelay(cfg.relayPin, false);
    st.pumpActive = false;
    st.pumpStopTime = currentTime;
    st.safetyTripped = true;
    Serial.print(F("[SAFETY] Max run time exceeded for "));
    Serial.print(cfg.name);
    Serial.println(F(". Pump stopped to prevent flooding."));
    return;
  }

  // Safety cooldown: Do not restart pump until minimum rest time has passed
  bool inCooldown = (currentTime - st.pumpStopTime < MIN_REST_TIME) && (st.pumpStopTime != 0);

  // Hysteresis control:
  // Turn pump ON only if soil is drier than start threshold AND we are not in cooldown
  if (!st.pumpActive && (st.moisturePct < cfg.startMoisturePct)) {
    if (!inCooldown && !st.safetyTripped) {
      setRelay(cfg.relayPin, true);
      st.pumpActive = true;
      st.pumpStartTime = currentTime;
      Serial.print(F("[ACTION] Soil dry ("));
      Serial.print(st.moisturePct);
      Serial.print(F("%). Turning ON pump for "));
      Serial.println(cfg.name);
    }
  }
  // Turn pump OFF when moisture reaches or exceeds stop threshold
  else if (st.pumpActive && (st.moisturePct >= cfg.stopMoisturePct)) {
    setRelay(cfg.relayPin, false);
    st.pumpActive = false;
    st.pumpStopTime = currentTime;
    st.safetyTripped = false; // Reset safety trip if normal moisture achieved
    Serial.print(F("[ACTION] Target moisture reached ("));
    Serial.print(st.moisturePct);
    Serial.print(F("%). Turning OFF pump for "));
    Serial.println(cfg.name);
  }
  // Between start and stop thresholds, the state stays as-is (hysteresis)
}

void logStatus(unsigned long currentTime) {
  Serial.print(F("TIME: "));
  Serial.print(currentTime / 1000);
  Serial.println(F("s | STATUS UPDATE:"));

  for (int i = 0; i < NUM_ZONES; i++) {
    Serial.print(F("  - "));
    Serial.print(zones[i].name);
    Serial.print(F(": Raw="));
    Serial.print(zoneStates[i].rawReading);
    Serial.print(F(" | Moisture="));
    Serial.print(zoneStates[i].moisturePct);
    Serial.print(F("% | Relay="));
    Serial.print(zoneStates[i].pumpActive ? F("ON") : F("OFF"));
    if (zoneStates[i].safetyTripped) {
      Serial.print(F(" [SAFETY TRIPPED]"));
    }
    Serial.println();
  }
  Serial.println(F("---------------------------------------------"));
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 1000) {
    // Wait for serial on native USB boards like Leonardo/Nano 33
  }

  Serial.println(F("============================================="));
  Serial.println(F("  Smart Automated Irrigation System Starting "));
  Serial.println(F("  Author: Abbas Khan                         "));
  Serial.println(F("============================================="));

  for (int i = 0; i < NUM_ZONES; i++) {
    pinMode(zones[i].sensorPin, INPUT);
    pinMode(zones[i].relayPin, OUTPUT);
    // Make sure relays start turned OFF
    setRelay(zones[i].relayPin, false);

    zoneStates[i].rawReading = 0;
    zoneStates[i].moisturePct = 0;
    zoneStates[i].pumpActive = false;
    zoneStates[i].safetyTripped = false;
    zoneStates[i].pumpStartTime = 0;
    zoneStates[i].pumpStopTime = 0;
  }

  Serial.println(F("System initialized in safe state. Starting monitoring."));
}

void loop() {
  unsigned long now = millis();

  // Read sensors at fixed non-blocking intervals
  if (now - lastReadTime >= SENSOR_READ_INTERVAL) {
    lastReadTime = now;

    for (int i = 0; i < NUM_ZONES; i++) {
      zoneStates[i].rawReading = averageReading(zones[i].sensorPin, NUM_SAMPLES);
      zoneStates[i].moisturePct = moistureToPercent(
        zoneStates[i].rawReading,
        zones[i].dryRaw,
        zones[i].wetRaw
      );
      controlZone(i, now);
    }
  }

  // Periodic telemetry log
  if (now - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = now;
    logStatus(now);
  }
}
