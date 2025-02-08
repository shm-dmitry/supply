#include "power_output_sense.h"

#include "Arduino.h"
#include "config.h"

// measured value for vref=2.5V
#define POWER_OUTPUT_SENSE_V_REF ((uint32_t)2490)

// measured value of R1 (2K7)
#define POWER_OUTPUT_SENSE_VV_R1 ((uint32_t)9120)
// measured value of R2 (1K)
#define POWER_OUTPUT_SENSE_VV_R2 ((uint32_t)988)

#define POWER_OUTPUT_SENSE_VI_GAIN ((uint32_t)50)
#define POWER_OUTPUT_SENSE_VI_1_div_RSHUNT ((uint32_t)100)

// I = (readI * vRef(2500 mV) / readVref) * (1/0.01) (1/rshunt) / G
#define POWER_OUTPUT_SENSE_VI_TO_I(value) (((((uint32_t)value) * POWER_OUTPUT_SENSE_V_REF * POWER_OUTPUT_SENSE_VI_1_div_RSHUNT) / ((uint32_t)power_output_vref_value)) / POWER_OUTPUT_SENSE_VI_GAIN)
// v = (readV * vRef (2500 mV) / readVref) * (9100(r1) + 1000(r2)) / 1000(r2)
#define POWER_OUTPUT_SENSE_VV_TO_V(value) ((((((uint32_t)value) * POWER_OUTPUT_SENSE_V_REF) / (uint32_t)power_output_vref_value) * (POWER_OUTPUT_SENSE_VV_R1 + POWER_OUTPUT_SENSE_VV_R2)) / POWER_OUTPUT_SENSE_VV_R2)

uint16_t power_output_sense_cache_i = 0;
uint16_t power_output_sense_cache_v = 0;
uint16_t power_output_vref_value = 0;

void power_output_sense_init() {
  analogReadResolution(10);
  analogSampleDuration(31);

  pinMode(PIN_POWER_OUTPUT_SENSE_I, INPUT);
  pinMode(PIN_POWER_OUTPUT_SENSE_V, INPUT);
  pinMode(PIN_POWER_OUTPUT_VREF, INPUT);

  power_output_sense_on_main_loop();
}

uint16_t power_output_sense_readI_x1000(bool readActual) {
  if (power_output_vref_value == 0) {
    return POWER_OUTPUT_SENSE_NOVALUE;
  }

  if (readActual) {
    power_output_sense_cache_i = POWER_OUTPUT_SENSE_VI_TO_I(analogRead(PIN_POWER_OUTPUT_SENSE_I));
  }
  
  return power_output_sense_cache_i;
}

uint16_t power_output_sense_readV_x1000(bool readActual) {
  if (power_output_vref_value == 0) {
    return POWER_OUTPUT_SENSE_NOVALUE;
  }

  if (readActual) {
    power_output_sense_cache_v = POWER_OUTPUT_SENSE_VV_TO_V(analogRead(PIN_POWER_OUTPUT_SENSE_V));
  }

  return power_output_sense_cache_v;
}

void power_output_sense_on_main_loop() {
  power_output_vref_value = analogRead(PIN_POWER_OUTPUT_VREF);
}

void power_output_sense_getraw(uint16_t * v, uint16_t * i, uint16_t * ref) {
  *v = analogRead(PIN_POWER_OUTPUT_SENSE_V);
  *i = analogRead(PIN_POWER_OUTPUT_SENSE_I);
  *ref = power_output_vref_value;
}


