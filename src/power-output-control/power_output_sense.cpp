#include "power_output_sense.h"

#include "Arduino.h"
#include "config.h"

#define POWER_OUTPUT_SENSE_MX_TO_X ((uint32_t)1000)
#define POWER_OUTPUT_SENSE_1024 ((uint32_t)1024)

#define POWER_OUTPUT_SENSE_V_OFFSET ((uint32_t)5)
#define POWER_OUTPUT_SENSE_VV_R1 ((uint32_t)9100)
#define POWER_OUTPUT_SENSE_VV_R2 ((uint32_t)1000)

#define POWER_OUTPUT_SENSE_VI_GAIN ((uint32_t)1000)
#define POWER_OUTPUT_SENSE_VI_1_div_RSHUNT ((uint32_t)100)
#define POWER_OUTPUT_SENSE_VI_RL ((uint32_t)47000)

// i = readI * 5 * 1000 (gain) * (1/0.01) (1/rshunt) / 1024 * 1000 (A to mA) / 47000
#define POWER_OUTPUT_SENSE_VI_TO_I(value) (((((uint32_t)value) * POWER_OUTPUT_SENSE_V_OFFSET * POWER_OUTPUT_SENSE_VI_GAIN * POWER_OUTPUT_SENSE_VI_1_div_RSHUNT / POWER_OUTPUT_SENSE_1024) * POWER_OUTPUT_SENSE_MX_TO_X) / POWER_OUTPUT_SENSE_VI_RL);
// v = readV * 5 * (9100(r1) + 1000(r2)) / 1000(r2) * 1000(V to mV) / 1024
#define POWER_OUTPUT_SENSE_VV_TO_V(value) ((((((uint32_t)value) * POWER_OUTPUT_SENSE_V_OFFSET * (POWER_OUTPUT_SENSE_VV_R1 + POWER_OUTPUT_SENSE_VV_R2)) / POWER_OUTPUT_SENSE_VV_R2) * POWER_OUTPUT_SENSE_MX_TO_X) / POWER_OUTPUT_SENSE_1024)

uint16_t power_output_sense_cache_i = 0;
uint16_t power_output_sense_cache_v = 0;

void power_output_sense_init() {
  pinMode(PIN_POWER_OUTPUT_SENSE_I, INPUT);
  pinMode(PIN_POWER_OUTPUT_SENSE_V, INPUT);
}

uint16_t power_output_sense_readI_x1000(bool readactual) {
  if (readactual) {
    power_output_sense_cache_i = POWER_OUTPUT_SENSE_VI_TO_I(analogRead(PIN_POWER_OUTPUT_SENSE_I));
  }
  
  return power_output_sense_cache_i;
}

uint16_t power_output_sense_readV_x1000(bool readactual) {
  if (readactual) {
    power_output_sense_cache_v = POWER_OUTPUT_SENSE_VV_TO_V(analogRead(PIN_POWER_OUTPUT_SENSE_V));
  }

  return power_output_sense_cache_v;
}
