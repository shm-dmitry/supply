#include "gui_pwm_page.h"

#include "gui_ipagecallback.h"
#include "user_input.h"

#include "pwm_control.h"

#include "Arduino.h"
#include "display.h"

#define GUI_MAIN_PAGE_DELTA_FREQ_SLOW 100
#define GUI_MAIN_PAGE_DELTA_FREQ_FAST 10000

#define GUI_MAIN_PAGE_DELTA_DUTY_SLOW 1
#define GUI_MAIN_PAGE_DELTA_DUTY_FAST 10

uint32_t gui_pwm_page_pwm_freq = 0;
uint8_t gui_pwm_page_pwm_duty = 0;
uint8_t gui_pwm_page_pwm_enabled = 0;

void gui_pwm_page_draw_static_lines();
void gui_pwm_page_draw_enabled(const t_pwm_control_status & status);
void gui_pwm_page_draw_freq(const t_pwm_control_status & status);
void gui_pwm_page_draw_duty(const t_pwm_control_status & status);
void gui_pwm_page_draw_time(const t_pwm_control_status & status);
void gui_pwm_page_draw_graph(const t_pwm_control_status & status);

void gui_pwm_page_on_refresh(uint8_t, uint8_t) {
  if (gui_pwm_page_pwm_freq == 0xFFFFFFFF) {
    gui_pwm_page_draw_static_lines();
  }

  t_pwm_control_status status;
  memset(&status, 0, sizeof(t_pwm_control_status));
  pwm_control_status(status);

  gui_pwm_page_draw_enabled(status);
  gui_pwm_page_draw_freq(status);
  gui_pwm_page_draw_duty(status);
  gui_pwm_page_draw_time(status);
  gui_pwm_page_draw_graph(status);

  gui_pwm_page_pwm_freq = status.freq;
  gui_pwm_page_pwm_duty = status.duty;
  gui_pwm_page_pwm_enabled = status.enabled;
}

void gui_pwm_page_on_reset(uint8_t, uint8_t) {
  gui_pwm_page_pwm_freq = 0xFFFFFFFF;
  gui_pwm_page_pwm_duty = 0xFF;
  gui_pwm_page_pwm_enabled = 0xFF;
}

void gui_pwm_page_draw_static_lines() {
  display_draw_rect(0, 0, DISPLAY_W, DISPLAY_H, DISPLAY_WHITE);

  display_set_textcolor(DISPLAY_GRAY);
  display_set_textsize(1);
  display_set_cursor(5, 5);

  display_prints("PWM");

  display_draw_line(3, 15, DISPLAY_W - 6, 15, DISPLAY_WHITE);

  display_set_cursor(25, 18);
  display_prints("Freq");

  display_set_cursor(110, 18);
  display_prints("Duty");

  display_set_cursor(90, 18 + 8*2);
  display_prints("Hz");

  display_set_cursor(150, 18 + 8*2);
  display_prints("%");

  display_draw_line(3, 45, DISPLAY_W - 6, 45, DISPLAY_WHITE);

  display_set_cursor(5, 47);
  display_prints("Ton+Toff");

  display_set_cursor(5, 47 + 10);
  display_prints("Ton");

  display_set_cursor(5, 47 + 20);
  display_prints("Toff");

  display_draw_line(3, 47+20+10, DISPLAY_W - 6, 47+20+10, DISPLAY_WHITE);
}

void gui_pwm_page_draw_enabled(const t_pwm_control_status & status) {
  if (gui_pwm_page_pwm_enabled == status.enabled) {
    return;
  }

  if (gui_pwm_page_pwm_enabled != 0xFF) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_textsize(1);
    display_set_cursor(45, 5);

    display_prints((gui_pwm_page_pwm_enabled == 0x01) ? "ENABLED" : "DISABLED");
  }

  display_set_textcolor(status.enabled ? DISPLAY_GREEN : DISPLAY_WHITE);
  display_set_textsize(1);
  display_set_cursor(45, 5);

  display_prints(status.enabled ? "ENABLED" : "DISABLED");
}

uint16_t gui_pwm_page_freq_to_offset(uint32_t freq) {
  char s[10] = {0};
  ltoa(freq, s, 10);
  size_t len = strlen(s);

  return 5 + 12*(7 - len);
}

void gui_pwm_page_draw_freq(const t_pwm_control_status & status) {
  if (gui_pwm_page_pwm_freq == status.freq) {
    return;
  }

  if (gui_pwm_page_pwm_freq != 0xFFFFFFFF) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_textsize(2);
    display_set_cursor(gui_pwm_page_freq_to_offset(gui_pwm_page_pwm_freq), 18+8+2);

    display_print32(gui_pwm_page_pwm_freq);
  }

  display_set_textcolor(DISPLAY_GREEN);
  display_set_textsize(2);
  display_set_cursor(gui_pwm_page_freq_to_offset(status.freq), 18+8+2);

  display_print32(status.freq);
}

void gui_pwm_page_draw_duty(const t_pwm_control_status & status) {
  if (gui_pwm_page_pwm_duty == status.duty) {
    return;
  }

  if (gui_pwm_page_pwm_duty != 0xFF) {
    display_set_textcolor(DISPLAY_BLACK);
    display_set_textsize(2);
    display_set_cursor(115, 18+8+2);

    display_print8(gui_pwm_page_pwm_duty);
  }

  display_set_textcolor(DISPLAY_GREEN);
  display_set_textsize(2);
  display_set_cursor(115, 18+8+2);

  display_print8(status.duty);
}

void gui_pwm_page_print_time(uint16_t x, uint16_t y, uint32_t timeNS, uint16_t color) {
  display_set_cursor(x, y);
  display_set_textsize(1);
  display_set_textcolor(color);

  if (timeNS == 0xFFFFFFFF) {
    display_prints("---");
  } else if (timeNS < 1000) { // nanoseconds
    display_print32(timeNS);
    display_prints(" ns");
  } else if (timeNS < 10000) { // <10 uS
    display_print32(timeNS / 1000);
    display_prints(".");
    if ((timeNS % 1000) / 10 < 10) {
      display_print8(0);
    }
    display_print32((timeNS % 1000) / 10);
    display_prints(" us");
  } else if (timeNS < 100000) { // <100 uS
    display_print32(timeNS / 1000);
    display_prints(".");
    display_print32((timeNS % 1000) / 100);
    display_prints(" us");
  } else if (timeNS < 1000000) { // < 1ms
    display_print32(timeNS / 1000);
    display_prints(" us");
  } else if (timeNS < 1000000000) { // < 1s
    display_print32(timeNS / 1000000);
    display_prints(" ms");
  } else {
    display_print32(timeNS / 1000000000);
    display_prints(".");
    display_print32((timeNS % 1000000000) / 100);
    display_prints(" s");
  }
}

void gui_pwm_page_draw_time(const t_pwm_control_status & status) {
  if (gui_pwm_page_pwm_duty == status.duty && gui_pwm_page_pwm_freq == status.freq) {
    return;
  }

  uint32_t time = (gui_pwm_page_pwm_freq == 0 ? 0xFFFFFFFF : (1000000000 / gui_pwm_page_pwm_freq));
  gui_pwm_page_print_time(70, 47, time, DISPLAY_BLACK);
  gui_pwm_page_print_time(70, 47+10, ((time == 0xFFFFFFFF) ? 0xFFFFFFFF : time * gui_pwm_page_pwm_duty / 100), DISPLAY_BLACK);
  gui_pwm_page_print_time(70, 47+20, ((time == 0xFFFFFFFF) ? 0xFFFFFFFF : time * (100 - gui_pwm_page_pwm_duty) / 100), DISPLAY_BLACK);

  time = (status.freq == 0 ? 0xFFFFFFFF : (1000000000 / status.freq));
  gui_pwm_page_print_time(70, 47, time, DISPLAY_WHITE);
  gui_pwm_page_print_time(70, 47+10, ((time == 0xFFFFFFFF) ? 0xFFFFFFFF : time * status.duty / 100), DISPLAY_WHITE);
  gui_pwm_page_print_time(70, 47+20, ((time == 0xFFFFFFFF) ? 0xFFFFFFFF : time * (100 - status.duty) / 100), DISPLAY_WHITE);
}

void gui_pwm_page_draw_graph(const t_pwm_control_status & status) {
  if (gui_pwm_page_pwm_duty == status.duty && gui_pwm_page_pwm_enabled == status.enabled) {
    return;
  }

  const uint16_t yHI = 85; 
  const uint16_t yLO = 115; 

  if (gui_pwm_page_pwm_duty == 100) {
    display_draw_line(10, yHI, DISPLAY_W - 20, yHI, DISPLAY_BLACK);
  } else if (gui_pwm_page_pwm_duty == 0) {
    display_draw_line(10, yLO, DISPLAY_W - 20, yLO, DISPLAY_BLACK);
  } else {
    uint16_t x = 12 + ((DISPLAY_W - 20 - 12) * gui_pwm_page_pwm_duty) / 100;

    display_draw_line(10, yLO, 12, yLO, DISPLAY_BLACK);
    display_draw_line(12, yHI, 12, yLO, DISPLAY_BLACK);
    display_draw_line(12, yHI, x, yHI, DISPLAY_BLACK);
    display_draw_line(x, yHI, x, yLO, DISPLAY_BLACK);
    display_draw_line(x, yLO, DISPLAY_W - 20, yLO, DISPLAY_BLACK);
  }

  uint16_t color = status.enabled ? DISPLAY_YELLOW : DISPLAY_WHITE;

  if (status.duty == 100) {
    display_draw_line(10, yHI, DISPLAY_W - 20, yHI, color);
  } else if (status.duty == 0) {
    display_draw_line(10, yLO, DISPLAY_W - 20, yLO, color);
  } else {
    uint16_t x = 12 + ((DISPLAY_W - 20 - 12) * status.duty) / 100;

    display_draw_line(10, yLO, 12, yLO, color);
    display_draw_line(12, yHI, 12, yLO, color);
    display_draw_line(12, yHI, x, yHI, color);
    display_draw_line(x, yHI, x, yLO, color);
    display_draw_line(x, yLO, DISPLAY_W - 20, yLO, color);
  }
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
