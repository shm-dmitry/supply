#include "overloaded_led.h"

#include "Arduino.h"
#include "config.h"

bool overloaded_led_active = false;

void overloaded_led_init() {
  pinMode(OVERLOADED_PIN, OUTPUT);
  digitalWrite(OVERLOADED_PIN, LOW);
  overloaded_led_active = false;
}

void overloaded_led_set(bool overload) {
  if (overloaded_led_active != overload) {
    digitalWrite(OVERLOADED_PIN, overload ? HIGH : LOW);
    overloaded_led_active = overload;
  }
}
