#include "power_output_control.h"
#include "i2c_slave.h"

#include "config.h"

void setup() {
  power_output_control_init();
  i2c_slave_init();

  #if UART_ENABLED
  Serial.begin(9600);
  Serial.println("Ready!");
  #endif

  power_output_control_setI(100);
  power_output_control_setV(200);
  power_output_control_start(true);
}

void loop() {
  power_output_control_on_main_loop();
  #if UART_ENABLED
  i2c_slave_on_main_loop();
  #endif
}
