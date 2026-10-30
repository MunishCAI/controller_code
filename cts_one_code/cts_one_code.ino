// cts_controller.ino  -- single file (debouncing.h merged in)

#define tailendSensor 2
#define uvSensor      3
#define stichSensor   4

// ---- Tuning -------------------------------------------------------------
#define DEBOUNCE_LOW_MS    5UL   // LOW must be stable this long to count as a detection
#define DEBOUNCE_HIGH_MS   30UL  // HIGH must be stable this long before re-arming
#define DEBUG_PULSE_WIDTH  0     // set to 1 to print real pulse width (ms) of every detection

// ---- Sensor state -------------------------------------------------------
struct Sensor {
  uint8_t       pin;
  const char*   name;
  bool          lastReading;   // raw reading from previous loop
  bool          stable;        // debounced state
  unsigned long lastChange;    // time raw reading last changed
  unsigned long lowStart;      // time the LOW edge occurred (for debug)
};

Sensor sensors[] = {
  { tailendSensor, "Tailend Sensor Detected", HIGH, HIGH, 0, 0 },
  { uvSensor,      "UV Sensor Detected",      HIGH, HIGH, 0, 0 },
  { stichSensor,   "Stich Sensor Detected",   HIGH, HIGH, 0, 0 },
};
const uint8_t SENSOR_COUNT = sizeof(sensors) / sizeof(sensors[0]);

// Returns true once per detection (debounced HIGH -> LOW)
bool sensorTriggered(Sensor &s) {
  bool reading = digitalRead(s.pin);
  unsigned long now = millis();

  if (reading != s.lastReading) {          // raw edge: restart timer
    s.lastReading = reading;
    s.lastChange  = now;
  }

  if (reading != s.stable) {
    unsigned long need = (reading == LOW) ? DEBOUNCE_LOW_MS : DEBOUNCE_HIGH_MS;
    if ((now - s.lastChange) >= need) {
      s.stable = reading;
      if (reading == LOW) {
        s.lowStart = s.lastChange;
        return true;
      }
#if DEBUG_PULSE_WIDTH
      Serial.print(s.name);
      Serial.print(" | pulse width ms: ");
      Serial.println(s.lastChange - s.lowStart);
#endif
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    pinMode(sensors[i].pin, INPUT_PULLUP);
    sensors[i].lastReading = digitalRead(sensors[i].pin);
    sensors[i].stable      = sensors[i].lastReading;
    sensors[i].lastChange  = millis();
  }
}

void loop() {
  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    if (sensorTriggered(sensors[i])) {
      Serial.println(sensors[i].name);
    }
  }
}
