#ifndef POWER_OUTPUT_SENSE_H_
#define POWER_OUTPUT_SENSE_H_

#include "stdint.h"

#define POWER_OUTPUT_SENSE_NOVALUE 32767

void power_output_sense_init();
uint16_t power_output_sense_readI_x1000(bool readActual);
uint16_t power_output_sense_readV_x1000(bool readActual);
void power_output_sense_on_main_loop();
void power_output_sense_getraw(uint16_t * v, uint16_t * i, uint16_t * ref);

#endif /* #define POWER_OUTPUT_SENSE_H_ */
