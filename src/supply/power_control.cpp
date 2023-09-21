#include "power_control.h"

#include "fan_control.h"
#include "user_input.h"

#include "Arduino.h"
#include "Wire.h"

#include "CRC.h"

#include "config.h"

#define POWER_CONTROL_RESET       A1
#define POWER_CONTROL_ENABLE_PIN  9

#define POWER_CONTROL_DEFAULT_V  5000
#define POWER_CONTROL_DEFAULT_I  3000
#define POWER_CONTROL_DEFAULT_EN false

#define I2C_ADDRESS_POWER_OUTPUT 0x33
#define I2C_ADDRESS_USB          0x41
#define I2C_ADDRESS_5V_3V3       0x43

#define I2C_COMMAND_SET_LIMIT_V 0x01
#define I2C_COMMAND_SET_LIMIT_I 0x02
#define I2C_COMMAND_SET_ENABLE  0x03
#define I2C_COMMAND_ADD_LIMIT_V 0x04
#define I2C_COMMAND_ADD_LIMIT_I 0x05
#define I2C_COMMAND_SUB_LIMIT_V 0x06
#define I2C_COMMAND_SUB_LIMIT_I 0x07
#define I2C_COMMAND_INV_ENABLE  0x08

#define I2C_TIMEOUT_MS 10

#define I2C_WAIT_FOR_DATA() \
  while(!Wire.available()) { \
    if (millis() > timeout) { \
      return false; \
    } \
  }

void power_control_init() {
  pinMode(POWER_CONTROL_RESET, OUTPUT);
  digitalWrite(POWER_CONTROL_RESET, LOW);

  delay(50);

  pinMode(POWER_CONTROL_ENABLE_PIN, OUTPUT);
  digitalWrite(POWER_CONTROL_ENABLE_PIN, HIGH); // disable

  Wire.begin();
  Wire.setClock(100000);

  // configure power-control chip with default configuration
  power_control_set_V(POWER_CONTROL_DEFAULT_V);
  power_control_set_I(POWER_CONTROL_DEFAULT_I);
  power_control_set_enabled(POWER_CONTROL_DEFAULT_EN);
}

void power_control_on_init_done() {
  digitalWrite(POWER_CONTROL_RESET, HIGH);
  delay(10); // await power controller started up
}

void power_control_set_I(uint16_t mA) {
  uint8_t data[4] = {
    I2C_COMMAND_SET_LIMIT_I,
    (uint8_t)(mA / 0xFF),
    (uint8_t)(mA % 0xFF),
    0x00
  };

  uint8_t crc = calcCRC8(data, 3);

  data[3] = crc;

  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(data, 4);
  Wire.endTransmission(true);
}

void power_control_set_V(uint16_t mV) {
  uint8_t data[4] = {
    I2C_COMMAND_SET_LIMIT_V,
    (uint8_t)(mV / 0xFF),
    (uint8_t)(mV % 0xFF),
    0x00
  };

  uint8_t crc = calcCRC8(data, 3);

  data[3] = crc;

  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(data, 4);
  Wire.endTransmission(true);
}

void power_control_add_I(uint16_t mA, bool add) {
  uint8_t data[4] = {
    (add ? (uint8_t)I2C_COMMAND_ADD_LIMIT_I : (uint8_t)I2C_COMMAND_SUB_LIMIT_I),
    (uint8_t)(mA / 0xFF),
    (uint8_t)(mA % 0xFF),
    0x00
  };

  uint8_t crc = calcCRC8(data, 3);

  data[3] = crc;

  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(data, 4);
  Wire.endTransmission(true);
}

void power_control_add_V(uint16_t mV, bool add) {
  uint8_t data[4] = {
    (add ? (uint8_t)I2C_COMMAND_ADD_LIMIT_V : (uint8_t)I2C_COMMAND_SUB_LIMIT_V),
    (uint8_t)(mV / 0xFF),
    (uint8_t)(mV % 0xFF),
    0x00
  };

  uint8_t crc = calcCRC8(data, 3);

  data[3] = crc;

  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(data, 4);
  Wire.endTransmission(true);
}

void power_control_set_enabled(bool enable) {
  digitalWrite(POWER_CONTROL_ENABLE_PIN, enable ? LOW : HIGH);

  uint8_t data[4] = {
    I2C_COMMAND_SET_ENABLE,
    (enable ? (uint8_t)0x01 : (uint8_t)0x00),
    0x00,
    0x00
  };

  uint8_t crc = calcCRC8(data, 3);

  data[3] = crc;

  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(data, 4);
  Wire.endTransmission(true);
}

void power_control_inv_enabled() {
  bool enabled = digitalRead(POWER_CONTROL_ENABLE_PIN) == LOW;
  power_control_set_enabled(!enabled);
}

bool power_control_powerout_status(t_power_output_stats & status) {
  Wire.beginTransmission(I2C_ADDRESS_POWER_OUTPUT);
  Wire.write(0);
  Wire.endTransmission(false);
  Wire.requestFrom(I2C_ADDRESS_POWER_OUTPUT, 10, 1);

  unsigned long timeout = millis() + I2C_TIMEOUT_MS;

  uint8_t data[11];
  memset(data, 0, sizeof(data));

  for (uint8_t i = 0; i<10; i++) {
    I2C_WAIT_FOR_DATA();
    data[i] = Wire.read();
  }

  #if I2C_SIMUL_MEMORY
  ;
  #else
  uint8_t crc = calcCRC8(data, 10 - 1);
  if (crc != data[10 - 1]) {
//    Serial.print("BAD crc: ");
//    Serial.print(crc, HEX);
//    Serial.print(" / ");
//    Serial.println(data[9], HEX);
    return false;
  }
  #endif

  status.limit_I = data[0] * 0xFF + data[1];
  status.limit_V = data[2] * 0xFF + data[3];
  status.actual_I = data[4] * 0xFF + data[5];
  status.actual_V = data[6] * 0xFF + data[7];
  status.enabled = data[8] == 0x01;

  if (status.enabled) {
    fan_control_check_power(status.actual_I, status.actual_V);
  }

  return true;
}

bool power_control_ina3221_read_busv(uint8_t address, uint8_t channel, uint16_t & result) {
  Wire.beginTransmission(address);
  #if I2C_SIMUL_MEMORY
  Wire.write((uint8_t)((channel - 1) * 4));
  #else
  Wire.write((uint8_t)(1 + (channel - 1) * 2 + 1));
  #endif
  Wire.endTransmission(false);

  Wire.requestFrom(address, (uint8_t)2, 1);

  unsigned long timeout = millis() + I2C_TIMEOUT_MS;
  I2C_WAIT_FOR_DATA();
  result = Wire.read() << 8;
  I2C_WAIT_FOR_DATA();
  result += Wire.read();

  return true;
}

bool power_control_ina3221_read_i(uint8_t address, uint8_t channel, uint16_t & result) {
  Wire.beginTransmission(address);
  #if I2C_SIMUL_MEMORY
  Wire.write((uint8_t)((channel - 1) * 4 + 2));
  #else
  Wire.write((uint8_t)(1 + (channel - 1) * 2));
  #endif
  Wire.endTransmission(false);

  Wire.requestFrom(address, (uint8_t)2, 1);

  unsigned long timeout = millis() + I2C_TIMEOUT_MS;
  I2C_WAIT_FOR_DATA();
  result = Wire.read() << 8;
  I2C_WAIT_FOR_DATA();
  result += Wire.read();

  if (result > 0x28F) {
    return false; // WTF? current > 65A ??
  } else {
    result *= 100; // 1 / 0.01ohm
  }

  return true;
}

bool power_control_usbstatus(t_power_output_usb_stats & status) {
  if (!power_control_ina3221_read_busv(I2C_ADDRESS_USB, 1, status.usb_1_v)) {
    return false;
  }
  if (!power_control_ina3221_read_busv(I2C_ADDRESS_USB, 2, status.usb_2_v)) {
    return false;
  }
  if (!power_control_ina3221_read_busv(I2C_ADDRESS_USB, 3, status.usb_3_v)) {
    return false;
  }

  if (!power_control_ina3221_read_i(I2C_ADDRESS_USB, 1, status.usb_1_i)) {
    return false;
  }
  if (!power_control_ina3221_read_i(I2C_ADDRESS_USB, 2, status.usb_2_i)) {
    return false;
  }
  if (!power_control_ina3221_read_i(I2C_ADDRESS_USB, 3, status.usb_3_i)) {
    return false;
  }

  fan_control_check_power(status.usb_1_i, status.usb_1_v);
  fan_control_check_power(status.usb_2_i, status.usb_2_v);
  fan_control_check_power(status.usb_3_i, status.usb_3_v);

  return true;
}

bool power_control_5v_3v3_line_status(uint16_t & i_5v, uint16_t & i_3v3) {
  if (!power_control_ina3221_read_i(I2C_ADDRESS_5V_3V3, 1, i_5v)) {
    return false;
  }
  if (!power_control_ina3221_read_i(I2C_ADDRESS_5V_3V3, 2, i_3v3)) {
    return false;
  }

  fan_control_check_power(i_5v, 5000);
  fan_control_check_power(i_3v3, 3300);

  return true;
}

void power_control_on_main_loop() {
  if (user_input_onoff_pressed()) {
    power_control_inv_enabled();
  }
}
