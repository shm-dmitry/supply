#ifndef POWER_OUTPUT_SENSE_H_
#define POWER_OUTPUT_SENSE_H_

#include "stdint.h"

void power_output_sense_init();
uint16_t power_output_sense_readI_x1000();
uint16_t power_output_sense_readV_x1000();

#endif /* #define POWER_OUTPUT_SENSE_H_ */
