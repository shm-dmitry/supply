#include "i2c_slave.h"

#include "Arduino.h"
#include "Wire.h"
#include "power_output_control.h"

#define I2C_SLAVE_ADDRESS 0x13

#define I2C_COMMAND_UNKNOWN 0x00
#define I2C_COMMAND_SET_LIMIT_V 0x01
#define I2C_COMMAND_SET_LIMIT_I 0x02
#define I2C_COMMAND_SET_ENABLE  0x03

void i2c_slave_on_receive(int bytes);
void i2c_slave_on_request();

void i2c_slave_init() {
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(i2c_slave_on_receive);
  Wire.onRequest(i2c_slave_on_request);
}

void i2c_slave_on_receive(int) {
  uint8_t command = I2C_COMMAND_UNKNOWN;
  if (Wire.available()) {
    command = Wire.read();
  }

  uint16_t arg = 0;
  if (Wire.available()) {
    arg = Wire.read() * 0xFF;
  }
  if (Wire.available()) {
    arg += Wire.read();
  }

  if (command == I2C_COMMAND_SET_LIMIT_V) {
    power_output_control_setV(arg);
  } else if (command == I2C_COMMAND_SET_LIMIT_I) {
    power_output_control_setI(arg);
  } else if (command == I2C_COMMAND_SET_ENABLE) {
    power_output_control_start(arg == 0x01 ? true : false);
  }
}

void i2c_slave_on_request() {
  t_power_output_stats status;
  memset(&status, 0, sizeof(t_power_output_stats));
  power_output_control_status(status);

  Wire.write(status.limit_I % 0xFF);
  Wire.write(status.limit_I / 0xFF);
  Wire.write(status.limit_V % 0xFF);
  Wire.write(status.limit_V / 0xFF);
  Wire.write(status.actual_I % 0xFF);
  Wire.write(status.actual_I / 0xFF);
  Wire.write(status.actual_V % 0xFF);
  Wire.write(status.actual_V / 0xFF);
  Wire.write(status.enabled ? 0x01 : 0x00);
}

#ifdef SIMUL_ENABLED
void i2c_slave_on_main_loop() {
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
