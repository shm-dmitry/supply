#include "power_control.h"
#include "user_input.h"
#include "fan_control.h"
#include "gui_manager.h"
#include "display.h"
#include "pwm_control.h"

#include "Wire.h"

void setup() {
  Serial.begin(9600);
  delay(10);

  power_control_init();

  fan_control_init();
  pwm_control_init();
  user_input_init();
  display_init();
  gui_manager_init();

  power_control_on_init_done();

  user_input_start();
  display_restart();

  Serial.println("Chip ready");
}

void loop() {
  fan_control_on_main_loop();
  gui_manager_on_main_loop();
  power_control_on_main_loop();

  delay(10); // ~100Hz refresh screen rate, I think it's OK )
}
