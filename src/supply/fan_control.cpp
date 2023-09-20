#include "fan_control.h"

#include "Arduino.h"

#define FAN_CONTROL_ENABLE_PIN A0
#define FAN_CONTROL_STOP_TIMEOUT 10000

#define FAN_CONTROL_START_I 1000
#define FAN_CONTROL_START_P 10000


unsigned long fan_control_timeout = 0;

void fan_control_init() {
  pinMode(FAN_CONTROL_ENABLE_PIN, OUTPUT);
  digitalWrite(FAN_CONTROL_ENABLE_PIN, LOW);
}

void fan_control_check_power(uint16_t i, uint16_t v) {
  if (i > FAN_CONTROL_START_I) {
    if (fan_control_timeout == 0) {
      digitalWrite(FAN_CONTROL_ENABLE_PIN, HIGH);
    }

    fan_control_timeout = millis() + FAN_CONTROL_STOP_TIMEOUT;
  } else if ((uint32_t)i * (uint32_t)v > FAN_CONTROL_START_P) {
    if (fan_control_timeout == 0) {
      digitalWrite(FAN_CONTROL_ENABLE_PIN, HIGH);
    }

    fan_control_timeout = millis() + FAN_CONTROL_STOP_TIMEOUT;
  }
}

void fan_control_on_main_loop() {
  if (fan_control_timeout > 0) {
    if (millis() > fan_control_timeout) {
      digitalWrite(FAN_CONTROL_ENABLE_PIN, LOW);
      fan_control_timeout = 0;
    }
  }
}
