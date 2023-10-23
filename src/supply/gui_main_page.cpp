#include "gui_main_page.h"

#include "Arduino.h"

#include "pwm_control.h"
#include "user_input.h"
#include "gui_ipagecallback.h"
#include "power_control.h"
#include "gui_icons.h"

#include "display.h"

#define GUI_MAIN_PAGE_DELTA_SLOW 100
#define GUI_MAIN_PAGE_DELTA_FAST 1000

#define GUI_MAIN_PAGE_ERR_POWER_OUTPUT (_BV(0))
#define GUI_MAIN_PAGE_ERR_USB          (_BV(1))
#define GUI_MAIN_PAGE_ERR_SOCKETS      (_BV(2))

// powerV - 0 ... 50.0 - 2 bytes [100mV precision]
// powerI - 0 ... 10.0 - 1 byte  [100mA precision]
uint16_t gui_main_page_limit_V = 0;
uint8_t gui_main_page_limit_I = 0;
uint16_t gui_main_page_actual_V = 0;
uint8_t gui_main_page_actual_I = 0;
uint8_t gui_main_page_power_on = 0;

// usb[1..3] V - 0 ... 20.0 - 1 byte [100mV precision]
// usb[1..3] I - 0 ... 3.0 - 1 byte  [100mA precision]
uint8_t gui_main_page_usb_V[3] = { 0, 0, 0 };
uint8_t gui_main_page_usb_I[3] = { 0, 0, 0 };

// 5V socket out; 3.3V socket out I - with 8mA precision - 2 bytes
uint16_t gui_main_page_socket_5v_I  = 0;
uint16_t gui_main_page_socket_3v3_I = 0;

// PWM - freq - 2 bytes & duty 1 byte 
uint16_t gui_main_page_pwm_freq = 0;
uint8_t gui_main_page_pwm_duty = 0;
uint8_t gui_main_page_pwm_enabled = 0;

uint8_t gui_main_page_error_read = 0;

void gui_main_page_draw_static_lines();
void gui_main_page_draw_power_output();
void gui_main_page_draw_usb();
void gui_main_page_draw_socket();
void gui_main_page_draw_pwm();
void gui_main_page_write_number_dot_1(uint16_t x, uint16_t y, uint8_t text_size, uint16_t value, uint16_t color);

void gui_main_page_on_refresh(uint8_t, uint8_t) {
  if (gui_main_page_limit_V == 0xFFFF) {
    gui_main_page_draw_static_lines();
  }

  gui_main_page_draw_power_output();
  gui_main_page_draw_usb();
  gui_main_page_draw_socket();
  gui_main_page_draw_pwm();
}

void gui_main_page_on_reset(uint8_t, uint8_t) {
  gui_main_page_limit_V = 0xFFFF;
  gui_main_page_limit_I = 0xFF;
  gui_main_page_actual_V = 0xFFFF;
  gui_main_page_actual_I = 0xFF;
  gui_main_page_power_on = 0xFF;

  memset(gui_main_page_usb_V, 0xFF, 3);
  memset(gui_main_page_usb_I, 0xFF, 3);

  gui_main_page_socket_5v_I  = 0xFFFF;
  gui_main_page_socket_3v3_I = 0xFFFF;

  gui_main_page_pwm_freq = 0xFFFF;
  gui_main_page_pwm_duty = 0xFF;
  gui_main_page_pwm_enabled = 0xFF;

  gui_main_page_error_read = 0;
}

void gui_main_page_write_number_dot_1(uint16_t x, uint16_t y, uint8_t text_size, uint16_t value, uint16_t color) {
  display_set_cursor(x, y);
  display_set_textsize(text_size);
  display_set_textcolor(color);

  if (value < 100) {
    display_prints(" ");
  }
  display_print16(value / 10);
  display_prints(".");
  display_print8(value % 10);
}

void gui_main_page_write_current(uint16_t x, uint16_t y, uint8_t text_size, uint16_t value, uint16_t color) {
  display_set_cursor(x, y);
  display_set_textsize(text_size);
  display_set_textcolor(color);

  if (value >= 1000) {
    display_print16(value / 1000);
    display_prints(".");
    if (((value % 1000) / 10) < 10) {
      display_print8(0);
    }
    display_print16((value % 1000) / 10);

    display_set_textcolor(DISPLAY_GRAY);
    display_prints(" A");
  } else {
    display_print16(value);

    display_set_textcolor(DISPLAY_GRAY);
    display_prints(" mA");
  }
}

void gui_main_page_draw_static_lines() {
  display_set_textcolor(DISPLAY_GRAY);

  // power output block
  display_draw_rect(0, 0, DISPLAY_W, DISPLAY_H, DISPLAY_WHITE);
  display_draw_line(78, 2, 78, 18, DISPLAY_WHITE);
  display_draw_line(78, 20, 78, 36, DISPLAY_WHITE);
  display_set_textsize(2);
  display_set_cursor(60, 3);
  display_prints("V");
  display_set_cursor(60, 21);
  display_prints("A");

  display_set_cursor(145, 3);
  display_prints("V");
  display_set_cursor(145, 21);
  display_prints("A");

  display_draw_line(3, 39, DISPLAY_W - 6, 39, DISPLAY_WHITE);

  // 3.3V / 5V socket block
  display_set_textsize(1);
  display_set_cursor(60, 42);
  display_prints("Sockets");

  display_set_cursor(25, 47);
  display_prints("5V");
  display_set_cursor(115, 47);
  display_prints("3.3V");

  display_draw_line(78, 52, 78, 70, DISPLAY_WHITE);
  display_draw_line(3, 73, DISPLAY_W - 6, 73, DISPLAY_WHITE);

  // PWM block
  display_set_cursor(5, 78);
  display_prints("PWM");
  display_set_cursor(80, 98);
  display_prints("KHz");
  display_set_cursor(150, 98);
  display_prints("%");

  // USB block
  display_draw_rect(0, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 4, DISPLAY_W, GUI_ICONS_USB_HEIGHT + 4, DISPLAY_WHITE);
  display_draw_bitmap(2, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2, GUI_ICONS_USB, GUI_ICONS_USB_WIDTH, GUI_ICONS_USB_HEIGHT, DISPLAY_WHITE);
  display_set_textsize(1);
  display_set_cursor(GUI_ICONS_USB_WIDTH + 44, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2);
  display_prints("V");
  display_set_cursor(GUI_ICONS_USB_WIDTH + 44, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2 + 9);
  display_prints("A");

  display_draw_line(GUI_ICONS_USB_WIDTH + 57, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2, GUI_ICONS_USB_WIDTH + 57, DISPLAY_H - 3,  DISPLAY_WHITE);

  display_set_cursor(GUI_ICONS_USB_WIDTH + 87, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2);
  display_prints("V");
  display_set_cursor(GUI_ICONS_USB_WIDTH + 87, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2 + 9);
  display_prints("A");

  display_draw_line(GUI_ICONS_USB_WIDTH + 100, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2, GUI_ICONS_USB_WIDTH + 100, DISPLAY_H - 3,  DISPLAY_WHITE);

  display_set_cursor(GUI_ICONS_USB_WIDTH + 130, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2);
  display_prints("V");
  display_set_cursor(GUI_ICONS_USB_WIDTH + 130, DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2 + 9);
  display_prints("A");
}

uint16_t gui_main_page_power_v_to_color(const t_power_output_stats & status) {
  if (!status.enabled) {
    return DISPLAY_WHITE;
  }

  if (status.actual_V < ((status.limit_V * 9) / 10) || status.actual_V < status.limit_V - 500) {
    return DISPLAY_RED;
  }

  if (status.actual_V < 5500) {
    return DISPLAY_GREEN;
  } else {
    return DISPLAY_YELLOW;
  }
}

uint16_t gui_main_page_power_i_to_color(const t_power_output_stats & status) {
  if (status.actual_V < ((status.limit_V * 9) / 10) || status.actual_V < status.limit_V - 500) {
    return DISPLAY_RED;
  }

  if (status.actual_I < 2000) {
    return DISPLAY_GREEN;
  } else {
    return DISPLAY_YELLOW;
  }
}

void gui_main_page_draw_power_output() {
  t_power_output_stats status;
  memset(&status, 0, sizeof(t_power_output_stats));
  if (!power_control_powerout_status(status)) {
    if (!(gui_main_page_error_read & GUI_MAIN_PAGE_ERR_POWER_OUTPUT)) {
      display_fill_rect(80, 16, 3, 3, DISPLAY_RED);
      gui_main_page_error_read = gui_main_page_error_read | GUI_MAIN_PAGE_ERR_POWER_OUTPUT;
    }

    return;
  }

  if (gui_main_page_error_read & GUI_MAIN_PAGE_ERR_POWER_OUTPUT) {
      display_fill_rect(80, 16, 3, 3, DISPLAY_BLACK);
      gui_main_page_error_read &= ~(GUI_MAIN_PAGE_ERR_POWER_OUTPUT);
  }

  uint16_t v = status.actual_V / 100;
  if (v != gui_main_page_actual_V) {
    if (gui_main_page_actual_V != 0xFFFF) {
      gui_main_page_write_number_dot_1(5, 3, 2, gui_main_page_actual_V, DISPLAY_BLACK);
    }

    gui_main_page_write_number_dot_1(5, 3, 2, v, gui_main_page_power_v_to_color(status));

    gui_main_page_actual_V = v;
  }

  v = status.limit_V / 100;
  if (v != gui_main_page_limit_V) {
    if (gui_main_page_limit_V != 0xFFFF) {
      gui_main_page_write_number_dot_1(90, 3, 2, gui_main_page_limit_V, DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(90, 3, 2, v, DISPLAY_WHITE);

    gui_main_page_limit_V = v;
  }

  v = status.actual_I / 100;
  if (status.enabled && gui_main_page_power_on == 0x00) {
    display_set_cursor(12, 21);
    display_set_textsize(2);
    display_set_textcolor(DISPLAY_BLACK);
    display_prints("OFF");

    gui_main_page_power_on = 0x01;
  }

  if (status.enabled && v != gui_main_page_actual_I) {
    if (gui_main_page_actual_I != 0xFF) {
      gui_main_page_write_number_dot_1(5, 21, 2, gui_main_page_actual_I, DISPLAY_BLACK);
    }

    if (status.enabled) {
      gui_main_page_write_number_dot_1(5, 21, 2, v, gui_main_page_power_i_to_color(status));
    }
    gui_main_page_actual_I = v;
  }

  if (!status.enabled && gui_main_page_power_on != 0x00) {
    if (gui_main_page_actual_I != 0xFF) {
      gui_main_page_write_number_dot_1(5, 21, 2, gui_main_page_actual_I, DISPLAY_BLACK);
    }

    display_set_cursor(12, 21);
    display_set_textsize(2);
    display_set_textcolor(DISPLAY_WHITE);
    display_prints("OFF");

    gui_main_page_power_on = 0x00;
    gui_main_page_actual_I = 0xFF;
  }

  v = status.limit_I / 100;
  if (v != gui_main_page_limit_I) {
    if (gui_main_page_limit_I != 0xFF) {
      gui_main_page_write_number_dot_1(90, 21, 2, gui_main_page_limit_I, DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(90, 21, 2, v, DISPLAY_WHITE);

    gui_main_page_limit_I = v;
  }
}

inline uint16_t gui_main_page_usb_current_to_color(uint16_t i) {
  if (i < 100) {
    return DISPLAY_WHITE;
  } else if (i < 1500) {
    return DISPLAY_GREEN;
  } else {
    return DISPLAY_RED;
  }
}

void gui_main_page_draw_usb() {
  t_power_output_usb_stats status;
  memset(&status, 0, sizeof(t_power_output_usb_stats));
  if (!power_control_usbstatus(status)) {
    if (!(gui_main_page_error_read & GUI_MAIN_PAGE_ERR_USB)) {
      display_fill_rect(15, DISPLAY_H - 5, 3, 3, DISPLAY_RED);
      gui_main_page_error_read = gui_main_page_error_read | GUI_MAIN_PAGE_ERR_USB;
    }

    return;
  }

  if (gui_main_page_error_read & GUI_MAIN_PAGE_ERR_USB) {
      display_fill_rect(15, DISPLAY_H - 5, 3, 3, DISPLAY_BLACK);
      gui_main_page_error_read &= ~(GUI_MAIN_PAGE_ERR_USB);
  }

  const uint16_t y = DISPLAY_H - GUI_ICONS_USB_HEIGHT - 2;
  uint16_t x = 33;

  uint8_t v = status.usb_1_v / 100;
  if (v != gui_main_page_usb_V[0]) {
    if (gui_main_page_usb_V[0] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y, 1, gui_main_page_usb_V[0], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y, 1, v, DISPLAY_WHITE);

    gui_main_page_usb_V[0] = v;
  }

   v = status.usb_1_i / 100;
  if (v != gui_main_page_usb_I[0]) {
    if (gui_main_page_usb_I[0] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y + 9, 1, gui_main_page_usb_I[0], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y + 9, 1, v, gui_main_page_usb_current_to_color(status.usb_1_i));

    gui_main_page_usb_I[0] = v;
  }

  x = 77;

  v = status.usb_2_v / 100;
  if (v != gui_main_page_usb_V[1]) {
    if (gui_main_page_usb_V[1] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y, 1, gui_main_page_usb_V[1], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y, 1, v, DISPLAY_WHITE);

    gui_main_page_usb_V[1] = v;
  }

   v = status.usb_2_i / 100;
  if (v != gui_main_page_usb_I[1]) {
    if (gui_main_page_usb_I[1] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y + 9, 1, gui_main_page_usb_I[1], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y + 9, 1, v, gui_main_page_usb_current_to_color(status.usb_2_i));

    gui_main_page_usb_I[1] = v;
  }

  x = 120;

  v = status.usb_3_v / 100;
  if (v != gui_main_page_usb_V[2]) {
    if (gui_main_page_usb_V[2] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y, 1, gui_main_page_usb_V[2], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y, 1, v, DISPLAY_WHITE);

    gui_main_page_usb_V[2] = v;
  }

   v = status.usb_3_i / 100;
  if (v != gui_main_page_usb_I[2]) {
    if (gui_main_page_usb_I[2] != 0xFF) {
      gui_main_page_write_number_dot_1(x, y + 9, 1, gui_main_page_usb_I[2], DISPLAY_BLACK);
    }
    gui_main_page_write_number_dot_1(x, y + 9, 1, v, gui_main_page_usb_current_to_color(status.usb_3_i));

    gui_main_page_usb_I[2] = v;
  }
}

inline uint16_t gui_main_page_socket_current_to_color(uint16_t i) {
  if (i < 50) {
    return DISPLAY_WHITE;
  } else if (i < 700) {
    return DISPLAY_GREEN;
  } else if (i < 2000) {
    return DISPLAY_YELLOW;
  } else {
    return DISPLAY_RED;
  }
}

void gui_main_page_draw_socket() {
  uint16_t i_5v = 0;
  uint16_t i_3v3 = 0;
  if (!power_control_5v_3v3_line_status(i_5v, i_3v3)) {
    if (!(gui_main_page_error_read & GUI_MAIN_PAGE_ERR_SOCKETS)) {
      display_fill_rect(103, 46, 3, 3, DISPLAY_RED);
      gui_main_page_error_read = gui_main_page_error_read | GUI_MAIN_PAGE_ERR_SOCKETS;
    }

    return;
  }

  if (gui_main_page_error_read & GUI_MAIN_PAGE_ERR_SOCKETS) {
      display_fill_rect(103, 46, 3, 3, DISPLAY_BLACK);
      gui_main_page_error_read &= ~(GUI_MAIN_PAGE_ERR_SOCKETS);
  }

  if (gui_main_page_socket_5v_I != i_5v) {
    if (gui_main_page_socket_5v_I != 0xFFFF) {
      gui_main_page_write_current(2, 56, 2, gui_main_page_socket_5v_I, DISPLAY_BLACK);
    }

    gui_main_page_write_current(2, 56, 2, i_5v, gui_main_page_socket_current_to_color(i_5v));

    gui_main_page_socket_5v_I = i_5v;
  }

  if (gui_main_page_socket_3v3_I != i_3v3) {
    if (gui_main_page_socket_3v3_I != 0xFFFF) {
      gui_main_page_write_current(85, 56, 2, gui_main_page_socket_3v3_I, DISPLAY_BLACK);
    }

    gui_main_page_write_current(85, 56, 2, i_3v3, gui_main_page_socket_current_to_color(i_3v3));

    gui_main_page_socket_3v3_I = i_3v3;
  }
}

uint8_t gui_main_page_freq_x_offset(uint16_t freq) {
  uint8_t x = 10;
  if (freq < 10000) {
    x += 8;
  }
  if (freq < 1000) {
    x += 8;
  }
  if (freq < 100) {
    x += 8;
  }
  return x;
}

void gui_main_page_draw_pwm() {
  t_pwm_control_status status;
  memset(&status, 0, sizeof(t_pwm_control_status));
  pwm_control_status(status);

  display_set_textsize(1);

  uint8_t actualEn = (status.enabled ? 0x01 : 0x00);

  if (actualEn != gui_main_page_pwm_enabled) {
    if (gui_main_page_pwm_enabled != 0xFF) {
      display_set_cursor(40, 78);
      display_set_textcolor(DISPLAY_BLACK);
      display_prints((gui_main_page_pwm_enabled == 0x01) ? "ENABLED" : "DISABLED");
    }

    display_set_cursor(40, 78);
    display_set_textcolor(status.enabled ? DISPLAY_GREEN : DISPLAY_WHITE);
    display_prints(status.enabled ? "ENABLED" : "DISABLED");

    gui_main_page_pwm_enabled = actualEn;
  }

  if (gui_main_page_pwm_freq != status.freq / 100) {
    if (gui_main_page_pwm_freq != 0xFFFF) {
      gui_main_page_write_number_dot_1(gui_main_page_freq_x_offset(gui_main_page_pwm_freq), 90, 2, gui_main_page_pwm_freq, DISPLAY_BLACK);
    }

    gui_main_page_write_number_dot_1(gui_main_page_freq_x_offset(status.freq / 100), 90, 2, status.freq / 100, DISPLAY_WHITE);

    gui_main_page_pwm_freq = status.freq / 100;
  }

  if (gui_main_page_pwm_duty != status.duty) {
    display_set_textsize(2);

    if (gui_main_page_pwm_duty != 0xFF) {
      display_set_cursor(110, 90);
      display_set_textcolor(DISPLAY_BLACK);
      if (gui_main_page_pwm_duty < 100) {
        display_prints(" ");
      }
      if (gui_main_page_pwm_duty < 10) {
        display_print8(0);
      }
      display_print8(gui_main_page_pwm_duty);
    }

    display_set_cursor(110, 90);
    display_set_textcolor(DISPLAY_WHITE);
    if (status.duty < 100) {
      display_prints(" ");
    }
    if (status.duty < 10) {
      display_print8(0);
    }
    display_print8(status.duty);

    gui_main_page_pwm_duty = status.duty;
  }
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
