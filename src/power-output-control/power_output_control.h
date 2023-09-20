#ifndef POWER_OUTPUT_CONTROL_H_
#define POWER_OUTPUT_CONTROL_H_

#include "stdint.h"

typedef struct t_power_output_stats {
  uint16_t limit_I;
  uint16_t limit_V;
  uint16_t actual_I;
  uint16_t actual_V;
  bool enabled;
} t_power_output_stats;


void power_output_control_init();

void power_output_control_on_main_loop();

void power_output_control_addV(uint16_t delta, bool add);
void power_output_control_addI(uint16_t delta, bool add);
void power_output_control_setV(uint16_t voltage_x1000);
void power_output_control_setI(uint16_t current_x1000);
void power_output_control_start(bool enable);

void power_output_control_status(t_power_output_stats & status);

#endif /* #define POWER_OUTPUT_CONTROL_H_ */
