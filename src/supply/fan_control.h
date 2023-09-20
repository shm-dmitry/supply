#ifndef FAN_CONTROL_H_
#define FAN_CONTROL_H_

#include "stdint.h"

void fan_control_init();

void fan_control_check_power(uint16_t i, uint16_t v);
void fan_control_on_main_loop();

#endif /* #define FAN_CONTROL_H_ */
