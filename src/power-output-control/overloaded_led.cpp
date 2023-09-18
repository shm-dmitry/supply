#include "overloaded_led.h"

#include "Arduino.h"

// C2
#define OVERLOADED_PIN 2

void overloaded_led_init() {
  pinMode(OVERLOADED_PIN, OUTPUT);
  digitalWrite(OVERLOADED_PIN, LOW);
}

void overloaded_led_set(bool overload) {
  digitalWrite(OVERLOADED_PIN, overload ? HIGH : LOW);
}
