#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

#include "config.h"

void i2c_slave_init();

#ifdef SIMUL_ENABLED
void i2c_slave_on_main_loop();
#endif

#endif /* #define I2C_SLAVE_H_ */
