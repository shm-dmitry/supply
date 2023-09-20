#ifndef GUI_PWM_PAGE_H_
#define GUI_PWM_PAGE_H_

#include "stdint.h"

void gui_pwm_page_on_refresh(uint8_t, uint8_t);
void gui_pwm_page_on_reset(uint8_t, uint8_t);
void gui_pwm_page_on_action(uint8_t input_type, uint8_t userinput_status);

#endif /* GUI_PWM_PAGE_H_ */
