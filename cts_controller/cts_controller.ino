#include "debouncing.h"


#define tailendSensor 2
#define uvSensor      3
#define stichSensor   4

void setup() {
  Serial.begin(115200);
  
  pinMode(tailendSensor, INPUT_PULLUP);
  pinMode(uvSensor, INPUT_PULLUP);
  pinMode(stichSensor, INPUT_PULLUP);
}

void loop() {
  if (debouncer(tailendSensor)) {
    Serial.println("Tailend Sensor Detected");
  }
  if (debouncer(uvSensor)) {
    Serial.println("UV Sensor Detected");
  }
  if (debouncer(stichSensor)) {
    Serial.println("Stich Sensor Detected");
  }
}
