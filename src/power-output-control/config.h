#ifndef CONFIG_H_
#define CONFIG_H_

#define SIMUL_ENABLED false

// C2
#define OVERLOADED_PIN A2

#if SIMUL_ENABLED
  #define POWER_OUTPUT_SENSE_PIN_I A1
  #define POWER_OUTPUT_SENSE_PIN_V A2

  #define POWER_OUTPUT_FB A4
#else
  // A7
  #define POWER_OUTPUT_SENSE_PIN_I A7
  // B4
  #define POWER_OUTPUT_SENSE_PIN_V 4

  // C1
  #define POWER_OUTPUT_FB A1
#endif

#define I2C_SLAVE_ADDRESS 0x13

#endif /* #define CONFIG_H_ */
