#include "uart_console.h"

#include "Arduino.h"
#include "power_output_control.h"

#if UART_ENABLED
void uart_console_init() {
  Serial.begin(9600);
}

void uart_console_on_main_loop() {
  if (Serial.available()) {
    uint8_t v = Serial.read();
    if (v == 'I') {
      uint16_t value = 0;
      while(true) {
        if (!Serial.available()) {
          continue;
        }

        uint8_t tmp = Serial.read();
        if (tmp == '\r' || tmp == '\n') {
          break;
        }

        value = value * 10 + (tmp - '0');
      }
      power_output_control_setI(value);
      Serial.print("Change I OK :: ");
      Serial.println(value);
    } else if (v == 'V') {
      uint16_t value = 0;
      while(true) {
        if (!Serial.available()) {
          continue;
        }

        uint8_t tmp = Serial.read();
        if (tmp == '\r' || tmp == '\n') {
          break;
        }

        value = value * 10 + (tmp - '0');
      }
      power_output_control_setV(value);
      Serial.print("Change V OK :: ");
      Serial.println(value);
    } else if (v == 'E') {
      while(!Serial.available());

      uint16_t value = 0;
      while(true) {
        if (!Serial.available()) {
          continue;
        }

        uint8_t tmp = Serial.read();
        if (tmp == '\r' || tmp == '\n') {
          break;
        }

        value = value * 10 + (tmp - '0');
      }
      power_output_control_start(value == 0x01 ? true : false);
      Serial.print("Change EN OK :: ");
      Serial.println(value);
    } else if (v == 'S') {
      t_power_output_stats status;
      memset(&status, 0, sizeof(t_power_output_stats));
      power_output_control_status(status);

      Serial.print("V : ");
      Serial.print(status.actual_V);
      Serial.print(" / ");
      Serial.println(status.limit_V);

      Serial.print("I : ");
      Serial.print(status.actual_I);
      Serial.print(" / ");
      Serial.println(status.limit_I);

      Serial.print("Enabled: ");
      Serial.println(status.enabled);
    } else if (v != '\r' && v != '\n') {
      Serial.print("Unknown command: ");
      Serial.println(v);
    }
  }
}
#endif