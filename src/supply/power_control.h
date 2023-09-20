#ifndef POWER_CONTROL_H_
#define POWER_CONTROL_H_

#include "stdint.h"

#define POWEROUTPUT_MAX_I 10000
#define POWEROUTPUT_MAX_V 50000

typedef struct t_power_output_stats {
  uint16_t limit_I;
  uint16_t limit_V;
  uint16_t actual_I;
  uint16_t actual_V;
  bool enabled;
} t_power_output_stats;

typedef struct t_power_output_usb_stats {
  uint16_t usb_1_i;
  uint16_t usb_1_v;
  uint16_t usb_2_i;
  uint16_t usb_2_v;
  uint16_t usb_3_i;
  uint16_t usb_3_v;
} t_power_output_usb_stats;

void power_control_init();
void power_control_on_init_done();
void power_control_on_main_loop();

void power_control_add_I(uint16_t mA, bool add);
void power_control_add_V(uint16_t mV, bool add);
void power_control_set_I(uint16_t mA);
void power_control_set_V(uint16_t mV);
void power_control_set_enabled(bool enable);

bool power_control_powerout_status(t_power_output_stats & status);
bool power_control_usbstatus(t_power_output_usb_stats & status);
bool power_control_5v_3v3_line_status(uint16_t & i_5v, uint16_t & i_3v3);

#endif /* #define POWER_CONTROL_H_ */
