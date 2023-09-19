#ifndef CONFIG_H_
#define CONFIG_H_

#define SIMUL_ENABLED false

#if SIMUL_ENABLED
  #define POWER_OUTPUT_SENSE_PIN_I A1
  #define POWER_OUTPUT_SENSE_PIN_V A2

  #define POWER_OUTPUT_FB A4
#endif

#endif /* #define CONFIG_H_ */
