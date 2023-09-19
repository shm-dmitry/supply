#include "power_output_control.h"

#include "Arduino.h"

#include "overloaded_led.h"
#include "power_output_sense.h"

#include "config.h"

uint16_t power_output_control_I_x1000 = 0;
uint16_t power_output_control_V_x1000 = 0;
bool power_output_control_enabled = false;

bool power_output_control_out_enabled = false;

#ifndef POWER_OUTPUT_FB
// PC1
#define POWER_OUTPUT_FB 1
#endif

void power_output_control_init() {
  pinMode(POWER_OUTPUT_FB, OUTPUT);
  digitalWrite(POWER_OUTPUT_FB, HIGH); // disable output
  power_output_control_out_enabled = false;

  power_output_sense_init();
  overloaded_led_init();
}

void power_output_control_on_main_loop() {
  bool allowWork = true;

  if (power_output_control_V_x1000 == 0 || power_output_control_I_x1000 == 0 || power_output_control_enabled == false) {
    allowWork = false;
  } else {
    uint16_t current_I = power_output_sense_readI_x1000();
    uint16_t current_V = power_output_sense_readV_x1000();

    allowWork = current_V <= power_output_control_V_x1000 && current_I <= power_output_control_I_x1000;
  }

  if (power_output_control_out_enabled != allowWork) {
    digitalWrite(POWER_OUTPUT_FB, allowWork ? LOW : HIGH);
    power_output_control_out_enabled = allowWork;
  }
}

void power_output_control_status(t_power_output_stats & status) {
  status.limit_I = power_output_control_I_x1000;
  status.limit_V = power_output_control_V_x1000;
  status.actual_I = power_output_sense_readI_x1000();
  status.actual_V = power_output_sense_readV_x1000();
  status.enabled = power_output_control_enabled;
}

void power_output_control_setV(uint16_t voltage_x1000) {
  power_output_control_V_x1000 = voltage_x1000;
}

void power_output_control_setI(uint16_t current_x1000) {
  power_output_control_I_x1000 = current_x1000;
}

void power_output_control_start(bool enable) {
  power_output_control_enabled = enable;
}
