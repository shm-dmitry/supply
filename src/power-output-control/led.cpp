#include "led.h"

#include "Arduino.h"
#include "config.h"

bool led_overloaded_active = false;
bool led_poweroutput_active = false;

void led_init() {
  pinMode(PIN_LED_OVERLOADED, OUTPUT);
  digitalWrite(PIN_LED_OVERLOADED, LOW);
  
  pinMode(PIN_LED_POWEROUTPUT, OUTPUT);
  digitalWrite(PIN_LED_POWEROUTPUT, LOW);

  led_overloaded_active = false;
  led_poweroutput_active = false;
}

void led_set_overloaded(bool on) {
  if (led_overloaded_active != on) {
    digitalWrite(PIN_LED_OVERLOADED, on ? HIGH : LOW);
    led_overloaded_active = on;
  }
}

void led_set_poweroutput(bool on) {
  if (led_poweroutput_active != on) {
    digitalWrite(PIN_LED_POWEROUTPUT, on ? HIGH : LOW);
    led_poweroutput_active = on;
  }
}
