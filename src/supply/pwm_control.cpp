#include "pwm_control.h"

#include "Arduino.h"

// PB2
#define PWM_CONTROL_PIN 10

#define PWM_CONTROL_DEFAULT_FREQ 40000
#define PWM_CONTROL_DEFAULT_DUTY 20

#define PWM_CONTROL_MAX_FREQ 1000000

bool pwm_control_enabled = false;

void pwm_control_init() {
  pinMode(PWM_CONTROL_PIN, OUTPUT);
  digitalWrite(PWM_CONTROL_PIN, LOW);

  TCCR1A = _BV(WGM11) | _BV(WGM10);
  pwm_control_start(false);
  pwm_control_set(PWM_CONTROL_DEFAULT_FREQ, PWM_CONTROL_DEFAULT_DUTY);

  pwm_control_enabled = false;
}

void pwm_control_set(uint32_t freq, uint8_t duty) {
  if (freq == 0) {
    pwm_control_start(false);
    OCR1A = 0;
  } else {
    OCR1A = (uint32_t)F_CPU / (uint32_t)2 / (uint32_t)freq;
  }

  OCR1B = ((uint32_t)OCR1A * (uint32_t)duty) / (uint32_t)100;
  if (OCR1B == 0) {
    pwm_control_start(false);
  } else if (OCR1A == OCR1B && pwm_control_enabled) {
    pwm_control_start(true);
  }
}

void pwm_control_add_freq(uint16_t delta, bool add) {
  if (OCR1A == 0) {
    pwm_control_set(delta, 0);
  } else {
    uint16_t prevOCR1A = OCR1A;
    t_pwm_control_status status;
    pwm_control_status(status);
      
    while(prevOCR1A == OCR1A) {
      if (add) {
        status.freq += delta;
        if (status.freq > PWM_CONTROL_MAX_FREQ) {
          status.freq = PWM_CONTROL_MAX_FREQ;
        }
      } else {
        if (status.freq < (uint32_t)delta) {
          status.freq = 0;
        } else {
          status.freq -= delta;
        }
      }

      pwm_control_set(status.freq, status.duty);

      if (status.freq == 0 || status.freq == PWM_CONTROL_MAX_FREQ) {
        break;
      }
    }

    // correct duty
    uint8_t expected = status.duty;
    pwm_control_status(status);

    if (status.duty < expected) {
      OCR1B++;
    }
  }
}

void pwm_control_add_duty(uint8_t delta, bool add) {
  if (OCR1A == 0) {
    pwm_control_set(PWM_CONTROL_DEFAULT_FREQ, delta);
  } else {
    uint16_t saveOCR1B = OCR1B;

    t_pwm_control_status status;
    pwm_control_status(status);
    while (saveOCR1B == OCR1B) {
      if (add) {
        if (delta > 100) {
          delta = 100;
        }

        if (status.duty > 100 - delta) {
          status.duty = 100;
        } else {
          status.duty += delta;
        }
      } else {
        if (status.duty < delta) {
          status.duty = 0;
        } else {
          status.duty -= delta;
        }
      }

      pwm_control_set(status.freq, status.duty);

      if (status.duty == 100 || status.duty == 0) {
        break;
      }
    }
  }
}

void pwm_control_start() {
  pwm_control_start(TCCR1B == 0);
}

void pwm_control_start(bool enabled) {
  if (enabled) {
    if (OCR1A == 0 || OCR1B == 0) {
      return;
    }

    if (OCR1A == OCR1B) {
      TCCR1B = 0;
      digitalWrite(PWM_CONTROL_PIN, HIGH);
    } else {    
      digitalWrite(PWM_CONTROL_PIN, LOW);
      TCCR1B = _BV(WGM13) | _BV(CS10);
    }

    pwm_control_enabled = true;
  } else {
    TCCR1B = 0;
    digitalWrite(PWM_CONTROL_PIN, LOW);

    pwm_control_enabled = false;
  }
}

void pwm_control_status(t_pwm_control_status & status) {
  status.enabled = pwm_control_enabled;
  if (OCR1A == 0) {
    status.duty = 0;
    status.freq = 0;
  } else {
    status.freq =  (uint32_t)F_CPU / (uint32_t)2 / (uint32_t)OCR1A;
    status.duty = ((uint32_t)OCR1B * (uint32_t)100) / (uint32_t) OCR1A;
  }
}
