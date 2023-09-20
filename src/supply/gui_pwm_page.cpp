#include "gui_pwm_page.h"

#include "gui_ipagecallback.h"
#include "user_input.h"

#include "pwm_control.h"

#define GUI_MAIN_PAGE_DELTA_FREQ_SLOW 100
#define GUI_MAIN_PAGE_DELTA_FREQ_FAST 10000

#define GUI_MAIN_PAGE_DELTA_DUTY_SLOW 1
#define GUI_MAIN_PAGE_DELTA_DUTY_FAST 10

void gui_pwm_page_on_refresh(uint8_t, uint8_t) {

}

void gui_pwm_page_on_reset(uint8_t, uint8_t) {

}

void gui_pwm_page_on_action(uint8_t input_type, uint8_t userinput_status) {
  if (userinput_status == USER_INPUT_STATUS_CLICK) {
    pwm_control_start();
    return;
  }
  
  bool add = (userinput_status == USER_INPUT_STATUS_RIGHT || userinput_status == USER_INPUT_STATUS_RIGHT_FAST);

  if (input_type == IPAGECALLBACK_INPUT_TYPE_I) {
    pwm_control_add_freq((USER_INPUT_IS_FAST(userinput_status) ? GUI_MAIN_PAGE_DELTA_FREQ_FAST : GUI_MAIN_PAGE_DELTA_FREQ_SLOW), add);
  } else if (input_type == IPAGECALLBACK_INPUT_TYPE_V) {
    pwm_control_add_duty((USER_INPUT_IS_FAST(userinput_status) ? GUI_MAIN_PAGE_DELTA_DUTY_FAST : GUI_MAIN_PAGE_DELTA_DUTY_SLOW), add);
  }
}
