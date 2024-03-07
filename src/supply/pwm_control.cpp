#include "pwm_control.h"

#include "Arduino.h"
#include "SPI.h"
#include "config.h"
#include "Wire.h"

#define PWM_CONTROL_DAC_I2C_ADDRESS    0x60

#define PWM_CONTROL_PIN_SIGNAL_CS      9 // PB1
#define PWM_CONTROL_PIN_GEN_SHUTDOWN   0 // PD0
#define PWM_CONTROL_PIN_GEN_ALWAYS_ON  A1

#define PWM_CONTROL_DEFAULT_FREQ 40000
#define PWM_CONTROL_DEFAULT_DUTY 20

#define PWM_CONTROL_GENOUT_MINV  50
#define PWM_CONTROL_GENOUT_VPP   450
#define PWM_CONTROL_GENOUT_MAXV  (PWM_CONTROL_GENOUT_MINV + PWM_CONTROL_GENOUT_VPP)

#define PWM_CONTROL_MIN_FREQ 1
#define PWM_CONTROL_MAX_FREQ 1000000

SPISettings spisettings = SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE2);

uint32_t pwm_control_freq = PWM_CONTROL_DEFAULT_FREQ;
uint8_t pwm_control_duty = PWM_CONTROL_DEFAULT_DUTY;
bool pwm_control_enabled = false;

void pwm_contorl_reset_dac();
void pwm_contorl_reset_gen();
void pwm_control_reconfigure_dac();
void pwm_control_reconfigure_gen();
void pwm_control_apply();
void pwm_control_spi_write16(uint16_t value);
void pwm_control_spi_write32(uint16_t value1, uint16_t value2);
void pwm_control_ic2_write24(uint8_t value1, uint8_t value2, uint8_t value3);

void pwm_control_init() {
  pinMode(PWM_CONTROL_PIN_GEN_SHUTDOWN, OUTPUT);
  digitalWrite(PWM_CONTROL_PIN_GEN_SHUTDOWN, HIGH);

  pinMode(PWM_CONTROL_PIN_GEN_ALWAYS_ON, OUTPUT);
  digitalWrite(PWM_CONTROL_PIN_GEN_ALWAYS_ON, LOW);

  pinMode(PWM_CONTROL_PIN_SIGNAL_CS, OUTPUT);
  digitalWrite(PWM_CONTROL_PIN_SIGNAL_CS, HIGH);
}

void pwm_control_configure_startup() {
  pwm_control_apply();
}

void pwm_control_set(uint32_t freq, uint8_t duty) {
  if (freq <= PWM_CONTROL_MAX_FREQ && freq >= PWM_CONTROL_MIN_FREQ && duty <= 100) {
    pwm_control_freq = freq;
    pwm_control_duty = duty;

    if (pwm_control_enabled) {
      pwm_control_apply();
    }
  }
}

void pwm_control_add_freq(uint16_t delta, bool add) {
  if (add) {
    if (pwm_control_freq + delta < PWM_CONTROL_MAX_FREQ) {
      pwm_control_freq += delta;
    } else {
      pwm_control_freq = PWM_CONTROL_MAX_FREQ;
    }
  } else {
    if (pwm_control_freq > delta + PWM_CONTROL_MIN_FREQ) {
      pwm_control_freq -= delta;
    } else {
      pwm_control_freq = PWM_CONTROL_MIN_FREQ;
    }
  }

  if (pwm_control_enabled) {
    pwm_control_apply();
  }
}

void pwm_control_add_duty(uint8_t delta, bool add) {
  if (add) {
    if (pwm_control_duty + delta < 100) {
      pwm_control_duty += delta;
    } else {
      pwm_control_duty = 100;
    }
  } else {
    if (pwm_control_duty > delta) {
      pwm_control_duty -= delta;
    } else {
      pwm_control_duty = 0;
    }
  }

  if (pwm_control_enabled) {
    pwm_control_apply();
  }
}

void pwm_control_start() {
  pwm_control_start(!pwm_control_enabled);
}

void pwm_control_start(bool enabled) {
  if (pwm_control_enabled != enabled) {
    pwm_control_enabled = enabled;
    pwm_control_apply();
  }
}

void pwm_control_apply() {
  Serial.print("pwm_control_apply: ");
  Serial.print(pwm_control_freq);
  Serial.print("Hz; ");
  Serial.print(pwm_control_duty);
  Serial.print("%; ");
  Serial.println(pwm_control_enabled ? "enabled" : "disabled");
 if (pwm_control_enabled) {
    digitalWrite(PWM_CONTROL_PIN_GEN_SHUTDOWN, HIGH);

    if (pwm_control_duty == 100 || pwm_control_duty == 0) {
      pwm_contorl_reset_dac();
      pwm_contorl_reset_gen();
    } else {
      pwm_control_reconfigure_dac();
      pwm_control_reconfigure_gen();
    }

    digitalWrite(PWM_CONTROL_PIN_GEN_ALWAYS_ON, (pwm_control_duty == 100) ? HIGH : LOW);
    
    digitalWrite(PWM_CONTROL_PIN_GEN_SHUTDOWN, LOW);
  } else {
    digitalWrite(PWM_CONTROL_PIN_GEN_SHUTDOWN, HIGH);
    digitalWrite(PWM_CONTROL_PIN_GEN_ALWAYS_ON, LOW);

    pwm_contorl_reset_dac();
    pwm_contorl_reset_gen();
  }
}

void pwm_control_status(t_pwm_control_status & status) {
  status.duty = pwm_control_duty;
  status.freq = pwm_control_freq;
  status.enabled = pwm_control_enabled;
}

void pwm_contorl_reset_dac() {
  pwm_control_ic2_write24(_BV(6) | _BV(4) | _BV(3) | _BV(1), 0, 0);
}

void pwm_contorl_reset_gen() {
  pwm_control_spi_write16(_BV(8) | _BV(6) | _BV(7));
}

void pwm_control_reconfigure_dac() {
  uint16_t vout = PWM_CONTROL_GENOUT_MAXV - ((uint32_t)PWM_CONTROL_GENOUT_VPP * (uint32_t)pwm_control_duty / (uint32_t)100);
  uint16_t dacreg = ((uint32_t)vout * (uint32_t)4095) / (uint32_t)4096;

  Serial.print("dac vout = ");
  Serial.println(vout);

  pwm_control_ic2_write24(_BV(6) | _BV(4) | _BV(3), (dacreg >> 4), ((dacreg & 0b00001111) << 4));
}

void pwm_control_reconfigure_gen() {
  pwm_control_spi_write16(_BV(13) | _BV(8));

  uint32_t freqreg = (((uint64_t)pwm_control_freq) * ((uint64_t)268435456)) / (uint64_t)25000000;
  Serial.print("freqreg = ");
  Serial.println(freqreg);

  uint16_t freq0 = _BV(14) + (freqreg & 0b0011111111111111);
  uint16_t freq1 = _BV(14) + (freqreg >> 14);

  pwm_control_spi_write32(freq0, freq1);

  pwm_control_spi_write16(_BV(15) | _BV(14));

  pwm_control_spi_write16(_BV(13) | _BV(1));
}

void pwm_control_spi_write16(uint16_t value) {
  SPI.beginTransaction(spisettings);
  digitalWrite(PWM_CONTROL_PIN_SIGNAL_CS, LOW);

  SPI.transfer16(value);

  digitalWrite(PWM_CONTROL_PIN_SIGNAL_CS, HIGH);
  SPI.endTransaction();

  Serial.print("SPI write ");
  Serial.println(value, HEX);
}

void pwm_control_spi_write32(uint16_t value1, uint16_t value2) {
  SPI.beginTransaction(spisettings);
  digitalWrite(PWM_CONTROL_PIN_SIGNAL_CS, LOW);

  SPI.transfer16(value1);
  SPI.transfer16(value2);

  digitalWrite(PWM_CONTROL_PIN_SIGNAL_CS, HIGH);
  SPI.endTransaction();

  Serial.print("SPI write ");
  Serial.print(value1, HEX);
  Serial.print(" / ");
  Serial.println(value2, HEX);
}

void pwm_control_ic2_write24(uint8_t value1, uint8_t value2, uint8_t value3) {
  Wire.beginTransmission(PWM_CONTROL_DAC_I2C_ADDRESS);
  Wire.write(value1);
  Wire.write(value2);
  Wire.write(value3);
  Wire.endTransmission(true);

  Serial.print("I2C write: ");
  Serial.print(value1, HEX);
  Serial.print(" / ");
  Serial.print(value2, HEX);
  Serial.print(" / ");
  Serial.println(value3, HEX);
}


