#ifndef PWM_CONTROL_H_
#define PWM_CONTROL_H_

#include "stdint.h"


typedef struct t_pwm_control_status {
  uint32_t freq;
  uint8_t duty;
  bool enabled;
} t_pwm_control_status;

void pwm_control_init();
void pwm_control_configure_startup();
void pwm_control_set(uint32_t freq, uint8_t duty);
void pwm_control_add_freq(uint16_t delta, bool add);
void pwm_control_add_duty(uint8_t delta, bool add);
void pwm_control_start(bool enabled);
void pwm_control_start();
void pwm_control_status(t_pwm_control_status & status);

#endif /* PWM_CONTROL_H_ */
