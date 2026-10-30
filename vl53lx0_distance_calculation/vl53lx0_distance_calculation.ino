#include <Wire.h>
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }

  Wire.begin();  // Mega: SDA=20, SCL=21

  Serial.println(F("Initializing VL53L0X..."));
  if (!lox.begin()) {
    Serial.println(F("VL53L0X not found. Check wiring (SDA=20, SCL=21, VIN, GND)."));
    while (1) { delay(10); }
  }
  Serial.println(F("VL53L0X ready."));
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);  // true = print debug info

  // RangeStatus 4 = out of range / phase failure
  if (measure.RangeStatus != 4) {
    Serial.print(F("Distance: "));
    Serial.print(measure.RangeMilliMeter);
    Serial.println(F(" mm"));
  } else {
    Serial.println(F("Out of range"));
  }

  delay(100);
}
