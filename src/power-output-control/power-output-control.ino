#include "power_output_control.h"
#include "i2c_slave.h"

void setup() {
  power_output_control_init();
  i2c_slave_init();
}

void loop() {
  power_output_control_on_main_loop();
}
