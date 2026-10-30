#ifndef TAGS_H
#define TAGS_H

#include <Arduino.h>

const String knownTags[] = {
  "1F00CF921456",
  "1F00CF010CDD"
};

const uint8_t numKnownTags = sizeof(knownTags) / sizeof(knownTags[0]);

// Returns the array index of a matching tag, or -1 if not found
int findTagIndex(const String &tagID) {
  for (uint8_t i = 0; i < numKnownTags; i++) {
    if (knownTags[i] == tagID) {
      return i;
    }
  }
  return -1;
}

#endif
