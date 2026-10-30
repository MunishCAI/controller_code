/*
 * CONE DETECTION v2 - single VL53L0X, Arduino Mega 2560
 *
 * One cone = distance falls from the rim to a minimum (centre) and rises again.
 * Prints "0" once at the centre of every cone.
 *
 * Changes from v1:
 *  - median-of-3 filter removes single-sample spikes
 *  - a cone is counted ONLY after a confirmed rise (the "cone left abruptly"
 *    fallback is removed: it produced extra counts on the trailing edge)
 *  - a short sensor dropout inside a cone no longer splits it into two cones
 *    (a pass ends only after END_INVALID_FALL / END_INVALID invalid readings)
 *  - touching cones: after a count, a fall of NEW_CONE_DROP_MM from the highest
 *    point opens the next cone; it counts only with a real drop + confirmed rise
 *  - DEBUG_TRACE dumps raw readings of any pass that gives 0 or 3+ counts
 */
#include <Wire.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// ---------- Tunable settings ----------
const uint16_t VALID_MIN_MM      = 20;    // below this = unreliable reading
const uint16_t VALID_MAX_MM      = 400;   // above this = no cone (8191 / out of range / background)
const uint16_t VALLEY_MAX_MM     = 400;   // a cone centre must be at or below this (set ~ 20 mm above your
                                          // farthest real centre to reject false valleys; 400 = off)
const uint16_t MIN_DROP_MM       = 30;    // rim-to-centre drop needed for a real cone
const uint16_t RISE_CONFIRM_MM   = 10;    // rise above the minimum that confirms the centre was passed
const uint8_t  RISE_SAMPLES      = 2;     // consecutive samples above that rise (1 = fastest print, less noise-proof)
const uint16_t NEW_CONE_DROP_MM  = 20;    // after a count: fall from highest point opens the next cone (it still needs MIN_DROP_MM + a rise to count)
const uint8_t  END_INVALID       = 4;     // consecutive invalid readings = pass has left (cone already counted / idle)
const uint8_t  END_INVALID_FALL  = 8;     // same, but while a cone is still being measured (tolerates sensor dropouts)
const uint32_t TIMING_BUDGET_US  = 20000; // faster sampling (default ~33000). Min ~20000

// ---------- Debug ----------
const uint8_t  DEBUG_TRACE       = 0;     // 0 = off, 1 = dump passes with 0 or 3+ counts, 2 = dump every pass
const uint16_t TRACE_LEN         = 200;   // raw readings kept per pass (0 in the dump = invalid)

// ---------- State ----------
enum Phase : uint8_t { IDLE, FALLING, RISING };

Phase    phase      = IDLE;
bool     inPass     = false;    // something valid has been seen and the pass has not ended
uint16_t peakD      = 0;        // rim (highest point) before the current descent
uint16_t valleyD    = 0;        // lowest point of the current descent
uint16_t riseMax    = 0;        // highest point after a count
uint8_t  riseRun    = 0;
uint8_t  invalidRun = 0;
uint16_t coneCount  = 0;        // total since power-up

uint16_t mbuf[3];               // median filter window
uint8_t  mn         = 0;

uint16_t passCones  = 0;        // debug bookkeeping for the current pass
uint16_t passValid  = 0;
uint16_t trace[TRACE_LEN];
uint16_t traceN     = 0;

// ---------- Helpers ----------
uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) { uint16_t t = a; a = b; b = t; }
  if (b > c) b = c;
  return (a > b) ? a : b;
}

// Returns false until the window holds 3 samples (first 2 samples of a pass are only buffered)
bool filterSample(uint16_t d, uint16_t &out) {
  if (mn < 3) mbuf[mn++] = d;
  else { mbuf[0] = mbuf[1]; mbuf[1] = mbuf[2]; mbuf[2] = d; }
  if (mn < 3) return false;
  out = median3(mbuf[0], mbuf[1], mbuf[2]);
  return true;
}

void onDetected() {
  coneCount++;
  passCones++;
  Serial.println(F("0"));
}

void endPass() {
  if (DEBUG_TRACE) {
    bool dump = (DEBUG_TRACE == 2) || (passCones >= 3) || (passCones == 0 && passValid >= 8);
    if (dump) {
      Serial.print(F("# pass cones="));
      Serial.print(passCones);
      Serial.print(F(" samples="));
      Serial.print(traceN);
      Serial.print(F(" :"));
      for (uint16_t i = 0; i < traceN; i++) {
        Serial.print(' ');
        Serial.print(trace[i]);
      }
      Serial.println();
    }
  }
  phase = IDLE;
  inPass = false;
  mn = 0;
  riseRun = 0;
  invalidRun = 0;
  passCones = 0;
  passValid = 0;
  traceN = 0;
}

void startFalling(uint16_t peak, uint16_t d) {
  phase = FALLING;
  peakD = peak;
  valleyD = d;
  riseRun = 0;
}

void processValid(uint16_t f) {
  switch (phase) {
    case IDLE:
      startFalling(f, f);
      break;

    case FALLING:
      if (f > peakD) {                       // higher rim: restart the descent from here
        peakD = f;
        valleyD = f;
        riseRun = 0;
      } else if (f < valleyD) {              // still descending
        valleyD = f;
        riseRun = 0;
      } else if (f >= valleyD + RISE_CONFIRM_MM) {
        if (++riseRun >= RISE_SAMPLES) {
          if ((peakD - valleyD >= MIN_DROP_MM) && (valleyD <= VALLEY_MAX_MM)) {
            onDetected();                    // centre passed: count once
            phase = RISING;
            riseMax = f;
          } else {                           // shallow dip = noise, not a cone
            valleyD = f;
            riseRun = 0;
          }
        }
      } else {
        riseRun = 0;
      }
      break;

    case RISING:
      if (f > riseMax) {
        riseMax = f;
      } else if (riseMax - f >= NEW_CONE_DROP_MM) {
        startFalling(riseMax, f);            // next cone starts from the rim peak
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  Wire.begin();  // Mega: SDA=20, SCL=21

  Serial.println(F("Initializing VL53L0X..."));
  if (!lox.begin()) {
    Serial.println(F("VL53L0X not found. Check wiring (SDA=20, SCL=21, VIN, GND)."));
    while (1) { delay(10); }
  }
  lox.setMeasurementTimingBudgetMicroSeconds(TIMING_BUDGET_US);
  Serial.println(F("VL53L0X ready."));
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  uint16_t d = measure.RangeMilliMeter;
  bool valid = (measure.RangeStatus != 4) && (d >= VALID_MIN_MM) && (d <= VALID_MAX_MM);

  if (DEBUG_TRACE && (valid || inPass) && traceN < TRACE_LEN) {
    trace[traceN++] = valid ? d : 0;
  }

  if (valid) {
    invalidRun = 0;
    inPass = true;
    if (passValid < 65535) passValid++;
    uint16_t f;
    if (filterSample(d, f)) processValid(f);
  } else if (inPass) {
    invalidRun++;
    if (invalidRun >= (phase == FALLING ? END_INVALID_FALL : END_INVALID)) endPass();
  }
}
