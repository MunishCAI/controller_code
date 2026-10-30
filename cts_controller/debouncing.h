#ifndef DEBOUNCING_H
#define DEBOUNCING_H

#include <Arduino.h>

#define MAX_DEBOUNCE_PINS 10
#define DEBOUNCE_DELAY_MS 50

struct DebounceState {
  uint8_t pin = 255;          // 255 = unused slot
  bool lastReading = HIGH;
  bool stableState = HIGH;
  unsigned long lastDebounceTime = 0;
};

static DebounceState debounceStates[MAX_DEBOUNCE_PINS];
static uint8_t debounceCount = 0;

inline DebounceState* getDebounceState(uint8_t pin) {
  for (uint8_t i = 0; i < debounceCount; i++) {
    if (debounceStates[i].pin == pin) return &debounceStates[i];
  }
  if (debounceCount < MAX_DEBOUNCE_PINS) {
    DebounceState &s = debounceStates[debounceCount];
    s.pin = pin;
    s.lastReading = digitalRead(pin);
    s.stableState = s.lastReading;
    s.lastDebounceTime = millis();
    return &debounceStates[debounceCount++];
  }
  return nullptr; // no free slots left
}


inline bool debouncer(uint8_t pin) {
  DebounceState *state = getDebounceState(pin);
  if (!state) return false;

  bool reading = digitalRead(pin);

  if (reading != state->lastReading) {
    state->lastDebounceTime = millis();
  }

  bool triggered = false;
  if ((millis() - state->lastDebounceTime) > DEBOUNCE_DELAY_MS) {
    if (reading != state->stableState) {
      state->stableState = reading;
      if (state->stableState == LOW) {
        triggered = true;
      }
    }
  }

  state->lastReading = reading;
  return triggered;
}

#endif
