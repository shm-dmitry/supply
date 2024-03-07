#include "power_output_control.h"
#include "i2c_slave.h"
#include "config.h"
#include "uart_console.h"

void setup() {
  power_output_control_init();
  i2c_slave_init();

  #if UART_ENABLED
  uart_console_init();
  Serial.println("Ready!");
  #endif

  #if SET_DEFAULT_V_I_ON_START
  power_output_control_setI(100);
  power_output_control_setV(200);
  power_output_control_start(true);
  #endif
}

void loop() {
  power_output_control_on_main_loop();

  #if UART_ENABLED
  uart_console_on_main_loop();
  #endif
}
