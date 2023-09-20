#include "gui_main_page.h"

#include "pwm_control.h"
#include "user_input.h"
#include "gui_ipagecallback.h"
#include "power_control.h"

#define GUI_MAIN_PAGE_DELTA_SLOW 100
#define GUI_MAIN_PAGE_DELTA_FAST 1000

void gui_main_page_on_refresh(uint8_t, uint8_t) {

}

void gui_main_page_on_reset(uint8_t, uint8_t) {

}

void gui_main_page_on_action(uint8_t input_type, uint8_t userinput_status) {
  if (userinput_status == USER_INPUT_STATUS_CLICK) {
    pwm_control_start();
    return;
  } 
  
  uint16_t delta = (USER_INPUT_IS_FAST(userinput_status) ? GUI_MAIN_PAGE_DELTA_FAST : GUI_MAIN_PAGE_DELTA_SLOW);
  bool add = (userinput_status == USER_INPUT_STATUS_RIGHT || userinput_status == USER_INPUT_STATUS_RIGHT_FAST);

  if (input_type == IPAGECALLBACK_INPUT_TYPE_I) {
    power_control_add_I(delta, add);
  } else if (input_type == IPAGECALLBACK_INPUT_TYPE_V) {
    power_control_add_V(delta, add);
  }
}
