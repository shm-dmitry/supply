#include "power_output_control.h"
#include "i2c_slave.h"

#include "config.h"

void setup() {
  #ifdef SIMUL_ENABLED
  Serial.begin(9600);
  #endif
  
  power_output_control_init();
  i2c_slave_init();
}

void loop() {
  power_output_control_on_main_loop();
  #ifdef SIMUL_ENABLED
  i2c_slave_on_main_loop();
  #endif
}
