/*
 * Smart Automated Irrigation System - Distributed Control Node
 * Author: Asad
 * Target: Arduino Uno / Nano (ATmega328P)
 *
 * Description:
 * Central receiver and actuation node located near pumps and power relays.
 * Receives serial packets from the remote sensor node, parses moisture values,
 * evaluates hysteresis thresholds, executes safety timeouts, and controls
 * active-low relay switches.
 *
 * Expected packet format:
 * DATA,<ZONE_ID>,<RAW_VALUE>,<MOISTURE_PCT>\n
 */

const bool RELAY_ACTIVE_LOW = true;
const int NUM_ZONES = 2;
const uint8_t RELAY_PINS[NUM_ZONES] = {7, 8};

const int START_MOISTURE_PCT[NUM_ZONES] = {30, 35};
const int STOP_MOISTURE_PCT[NUM_ZONES] = {60, 65};

const unsigned long MAX_PUMP_RUN_TIME = 15000; // 15 seconds
const unsigned long MIN_REST_TIME = 8000;      // 8 seconds cooldown
const unsigned long PACKET_TIMEOUT = 10000;    // 10 seconds without data trips safety

struct NodeZoneState {
  int rawReading;
  int moisturePct;
  bool pumpActive;
  bool safetyTripped;
  unsigned long pumpStartTime;
  unsigned long pumpStopTime;
  unsigned long lastPacketTime;
};

NodeZoneState state[NUM_ZONES];
String rxBuffer = "";

void setRelay(uint8_t pin, bool turnOn) {
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(pin, turnOn ? LOW : HIGH);
  } else {
    digitalWrite(pin, turnOn ? HIGH : LOW);
  }
}

void processIncomingPacket(String line) {
  line.trim();
  if (line.length() == 0 || line.startsWith("#")) {
    return; // Ignore comments and empty lines
  }

  // Expecting format: DATA,zone,raw,pct
  if (!line.startsWith("DATA,")) {
    return;
  }

  int firstComma = line.indexOf(',');
  int secondComma = line.indexOf(',', firstComma + 1);
  int thirdComma = line.indexOf(',', secondComma + 1);

  if (firstComma == -1 || secondComma == -1 || thirdComma == -1) {
    return; // Malformed packet
  }

  int zoneId = line.substring(firstComma + 1, secondComma).toInt();
  int raw = line.substring(secondComma + 1, thirdComma).toInt();
  int pct = line.substring(thirdComma + 1).toInt();

  if (zoneId < 1 || zoneId > NUM_ZONES) {
    return; // Invalid zone
  }

  int idx = zoneId - 1;
  unsigned long now = millis();

  state[idx].rawReading = raw;
  state[idx].moisturePct = pct;
  state[idx].lastPacketTime = now;

  // Evaluate hysteresis and actuation
  bool inCooldown = (now - state[idx].pumpStopTime < MIN_REST_TIME) && (state[idx].pumpStopTime != 0);

  // Turn ON if dry and safe
  if (!state[idx].pumpActive && (pct < START_MOISTURE_PCT[idx])) {
    if (!inCooldown && !state[idx].safetyTripped) {
      setRelay(RELAY_PINS[idx], true);
      state[idx].pumpActive = true;
      state[idx].pumpStartTime = now;
      Serial.print(F("[CONTROL] Zone "));
      Serial.print(zoneId);
      Serial.print(F(" moisture dry ("));
      Serial.print(pct);
      Serial.println(F("%). Relay engaged."));
    }
  }
  // Turn OFF if saturated
  else if (state[idx].pumpActive && (pct >= STOP_MOISTURE_PCT[idx])) {
    setRelay(RELAY_PINS[idx], false);
    state[idx].pumpActive = false;
    state[idx].pumpStopTime = now;
    state[idx].safetyTripped = false;
    Serial.print(F("[CONTROL] Zone "));
    Serial.print(zoneId);
    Serial.print(F(" target reached ("));
    Serial.print(pct);
    Serial.println(F("%). Relay disengaged."));
  }
}

void checkSafetyTimeouts(unsigned long now) {
  for (int i = 0; i < NUM_ZONES; i++) {
    // Timeout 1: Remote sensor packet lost
    if (state[i].pumpActive && (now - state[i].lastPacketTime > PACKET_TIMEOUT)) {
      setRelay(RELAY_PINS[i], false);
      state[i].pumpActive = false;
      state[i].pumpStopTime = now;
      state[i].safetyTripped = true;
      Serial.print(F("[SAFETY WARNING] Zone "));
      Serial.print(i + 1);
      Serial.println(F(" telemetry lost while pump running. Shutting down."));
    }

    // Timeout 2: Pump maximum continuous run time exceeded
    if (state[i].pumpActive && (now - state[i].pumpStartTime >= MAX_PUMP_RUN_TIME)) {
      setRelay(RELAY_PINS[i], false);
      state[i].pumpActive = false;
      state[i].pumpStopTime = now;
      state[i].safetyTripped = true;
      Serial.print(F("[SAFETY WARNING] Zone "));
      Serial.print(i + 1);
      Serial.println(F(" max run duration exceeded. Safety cutoff activated."));
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 1000) {}

  for (int i = 0; i < NUM_ZONES; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    setRelay(RELAY_PINS[i], false);

    state[i].rawReading = 0;
    state[i].moisturePct = 0;
    state[i].pumpActive = false;
    state[i].safetyTripped = false;
    state[i].pumpStartTime = 0;
    state[i].pumpStopTime = 0;
    state[i].lastPacketTime = millis();
  }

  Serial.println(F("============================================="));
  Serial.println(F("  Smart Irrigation Distributed Control Node "));
  Serial.println(F("  Author: Asad                         "));
  Serial.println(F("============================================="));
}

void loop() {
  unsigned long now = millis();

  // Read serial input buffer
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      processIncomingPacket(rxBuffer);
      rxBuffer = "";
    } else if (c != '\r') {
      rxBuffer += c;
    }
  }

  // Check safety conditions
  checkSafetyTimeouts(now);
}
