#include "i2c_slave.h"

#include "Arduino.h"
#include "Wire.h"
#include "power_output_control.h"
#include "CRC.h"

#define I2C_COMMAND_SET_LIMIT_V 0x01
#define I2C_COMMAND_SET_LIMIT_I 0x02
#define I2C_COMMAND_SET_ENABLE  0x03
#define I2C_COMMAND_ADD_LIMIT_V 0x04
#define I2C_COMMAND_ADD_LIMIT_I 0x05
#define I2C_COMMAND_SUB_LIMIT_V 0x06
#define I2C_COMMAND_SUB_LIMIT_I 0x07

void i2c_slave_on_receive(int bytes);
void i2c_slave_on_request();

int x = 0;

void i2c_slave_init() {
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.setClock(100000);
  Wire.onReceive(&i2c_slave_on_receive);
  Wire.onRequest(&i2c_slave_on_request);
}

void i2c_slave_on_receive(int bytes) {
  x++;
  if (bytes != 4) {
    return;
  }

  if (Wire.available() < 4) {
    return;
  }
  uint8_t command = Wire.read();
  uint8_t arg1 = Wire.read();
  uint8_t arg2 = Wire.read();
  uint8_t crc = Wire.read();

  uint8_t data[3] = { command, arg1, arg2 };
  uint8_t calc_crc = calcCRC8(data, sizeof(data));

  if (crc == calc_crc) {
    if (command == I2C_COMMAND_SET_LIMIT_V) {
      power_output_control_setV(arg1 * 0xFF + arg2);
    } else if (command == I2C_COMMAND_SET_LIMIT_I) {
      power_output_control_setI(arg1 * 0xFF + arg2);
    } else if (command == I2C_COMMAND_SET_ENABLE) {
      power_output_control_start(arg1 == 0x01 ? true : false);
    } else if (command == I2C_COMMAND_ADD_LIMIT_V) {
      power_output_control_addV(arg1 * 0xFF + arg2, true);
    } else if (command == I2C_COMMAND_ADD_LIMIT_I) {
      power_output_control_addI(arg1 * 0xFF + arg2, true);
    } else if (command == I2C_COMMAND_SUB_LIMIT_V) {
      power_output_control_addV(arg1 * 0xFF + arg2, false);
    } else if (command == I2C_COMMAND_SUB_LIMIT_I) {
      power_output_control_addI(arg1 * 0xFF + arg2, false);
    }
  }
}

void i2c_slave_on_request() {
  t_power_output_stats status;
  memset(&status, 0, sizeof(t_power_output_stats));
  power_output_control_status(status);

  uint8_t data[12] = { 
    (uint8_t)(status.limit_I / 0xFF),
    (uint8_t)(status.limit_I % 0xFF),
    (uint8_t)(status.limit_V / 0xFF),
    (uint8_t)(status.limit_V % 0xFF),
    (uint8_t)(status.actual_I / 0xFF),
    (uint8_t)(status.actual_I % 0xFF),
    (uint8_t)(status.actual_V / 0xFF),
    (uint8_t)(status.actual_V % 0xFF),
    (status.enabled ? (uint8_t)0x01 : (uint8_t)0x00),
    0x00,
    0x00
   };

  uint8_t crc = calcCRC8(data, 10 - 1);

  data[10 - 1] = crc;
/*
  for (uint8_t i = 0; i<10; i++) {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  */
  Wire.write(data, 12);
}

#if UART_ENABLED
void i2c_slave_on_main_loop() {
  if (x > 0) {
    Serial.println("CALLBACK!!");
  }

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
