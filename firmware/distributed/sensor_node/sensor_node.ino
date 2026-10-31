/*
 * Smart Automated Irrigation System - Distributed Sensor Node
 * Author: Abbas Khan
 * Target: Arduino Uno / Nano (ATmega328P)
 *
 * Description:
 * Dedicated transmitter node deployed out in the field/garden bed.
 * Continuously samples analog soil moisture sensors, performs sample averaging,
 * computes moisture percentages, and transmits formatted telemetry packets
 * over Serial (UART) to the central control node.
 *
 * Protocol Format:
 * DATA,<ZONE_ID>,<RAW_VALUE>,<MOISTURE_PCT>\n
 * Example: DATA,1,815,14
 */

const int NUM_ZONES = 2;
const uint8_t SENSOR_PINS[NUM_ZONES] = {A0, A1};
const int DRY_CAL[NUM_ZONES] = {850, 850};
const int WET_CAL[NUM_ZONES] = {320, 320};

const int NUM_SAMPLES = 5;
const unsigned long TRANSMIT_INTERVAL = 2000; // 2 seconds between packets
const uint8_t HEARTBEAT_LED = 13;

unsigned long lastTransmitTime = 0;
bool heartbeatState = false;

int readAndAverage(uint8_t pin, int samples) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  return (int)(sum / samples);
}

int calculateMoisturePercent(int raw, int dryVal, int wetVal) {
  int clamped = constrain(raw, min(wetVal, dryVal), max(wetVal, dryVal));
  long pct = map(clamped, dryVal, wetVal, 0, 100);
  return (int)constrain(pct, 0, 100);
}

void setup() {
  Serial.begin(9600);
  pinMode(HEARTBEAT_LED, OUTPUT);
  for (int i = 0; i < NUM_ZONES; i++) {
    pinMode(SENSOR_PINS[i], INPUT);
  }

  // Startup banner for debugging
  Serial.println(F("# SENSOR_NODE_ONLINE:v1.0"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastTransmitTime >= TRANSMIT_INTERVAL) {
    lastTransmitTime = now;

    // Toggle heartbeat LED on each transmission
    heartbeatState = !heartbeatState;
    digitalWrite(HEARTBEAT_LED, heartbeatState ? HIGH : LOW);

    // Read and transmit each zone packet
    for (int i = 0; i < NUM_ZONES; i++) {
      int raw = readAndAverage(SENSOR_PINS[i], NUM_SAMPLES);
      int pct = calculateMoisturePercent(raw, DRY_CAL[i], WET_CAL[i]);

      // Packet: DATA,<ZONE>,<RAW>,<PCT>
      Serial.print(F("DATA,"));
      Serial.print(i + 1);
      Serial.print(F(","));
      Serial.print(raw);
      Serial.print(F(","));
      Serial.println(pct);
    }
  }
}
