#include "gui_manager.h"
#include "gui_ipagecallback.h"
#include "user_input.h"
#include "display.h"
#include "stddef.h"
#include "Arduino.h"

#include "gui_main_page.h"
#include "gui_pwm_page.h"

#define GUI_PAGE_ACTION_ONREFRESH  0
#define GUI_PAGE_ACTION_ONRESET    1
#define GUI_PAGE_ACTION_ONACTION   2

#define GUI_MANAGER_START_PAGE     0

#define PAGES_COUNT    2
#define PAGE_CALLBACKS (GUI_PAGE_ACTION_ONACTION + 1)

uint8_t gui_page_current = 0;

// this "vtable" saves RAM and ROM against classes with virtual methods
const t_page_callback PAGES_VTABLE[PAGES_COUNT][PAGE_CALLBACKS] = {
  {&gui_main_page_on_refresh, &gui_main_page_on_reset, &gui_main_page_on_action},
  {&gui_pwm_page_on_refresh,  &gui_pwm_page_on_reset,  &gui_pwm_page_on_action}
};

void gui_manager_init() {
  gui_page_current = 0xFF;
}

void gui_manager_on_main_loop() {
  bool refresh = false;

  if (gui_page_current == 0xFF) {
    gui_page_current = GUI_MANAGER_START_PAGE;
    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONRESET];
    if (func != NULL) {
      func(0x00, 0x00);
    }

    refresh = true;
  }

  uint8_t istatus = user_input_encoder_i_status();
  uint8_t vstatus = user_input_encoder_v_status();
  if (vstatus == USER_INPUT_STATUS_CLICK && istatus == USER_INPUT_STATUS_CLICK) {
    display_restart();
  }

  if (vstatus == USER_INPUT_STATUS_CLICK) {
    gui_page_current++;
    if (gui_page_current >= PAGES_COUNT) {
      gui_page_current = 0;
    }

    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONRESET];
    if (func != NULL) {
      func(0x00, 0x00);
    }

    refresh = true;

    vstatus = USER_INPUT_STATUS_NONE;
  }

  if (refresh) {
    display_fill_rect(0, 0, DISPLAY_W, DISPLAY_H, DISPLAY_BLACK);
  }

  if (vstatus != USER_INPUT_STATUS_NONE) {
    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONACTION];
    if (func != NULL) {
      func(IPAGECALLBACK_INPUT_TYPE_V, vstatus);
    }
  }

  if (istatus != USER_INPUT_STATUS_NONE) {
    t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONACTION];
    if (func != NULL) {
      func(IPAGECALLBACK_INPUT_TYPE_I, istatus);
    }
  }

  t_page_callback func = PAGES_VTABLE[gui_page_current][GUI_PAGE_ACTION_ONREFRESH];
  if (func != NULL) {
    func(0x00, 0x00);
  }
}
