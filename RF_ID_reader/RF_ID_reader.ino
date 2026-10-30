#include "tags.h"
/*
VCC	5V
GND	GND
TX	19 (RX1)
*/
#define RFID_SERIAL Serial1

String tagID = "";
unsigned long lastCharTime = 0;
const unsigned long TAG_TIMEOUT_MS = 50;
const uint8_t TAG_LENGTH = 12;

void setup() {
  Serial.begin(9600);
  RFID_SERIAL.begin(9600);
  Serial.println("RFID Reader Ready...");
}

void loop() {
  while (RFID_SERIAL.available()) {
    char c = RFID_SERIAL.read();
    tagID += c;
    lastCharTime = millis();
  }

  if (tagID.length() > 0 && (millis() - lastCharTime) > TAG_TIMEOUT_MS) {
    processTag(tagID);
    tagID = "";
  }
}

void processTag(String id) {
  id.trim();
  if (id.length() == TAG_LENGTH) {
    int idx = findTagIndex(id);
    if (idx >= 0) {
      Serial.print("Known tag — index ");
      Serial.print(idx);
      Serial.print(" | UID: ");
      Serial.println(id);
    } else {
      Serial.print("Unknown tag — UID: ");
      Serial.println(id);
    }
  }
}
