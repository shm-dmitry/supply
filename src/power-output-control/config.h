#ifndef CONFIG_H_
#define CONFIG_H_

#define UART_ENABLED true

#define EXT_CLOCK_ENABLED false

// C2
#define OVERLOADED_PIN 12

// A7
#define POWER_OUTPUT_SENSE_PIN_I A7
// B4
//#define POWER_OUTPUT_SENSE_PIN_V 4
#define POWER_OUTPUT_SENSE_PIN_V A9

// C1
#define POWER_OUTPUT_FB 11

#define I2C_SLAVE_ADDRESS 0x33

#endif /* #define CONFIG_H_ */
