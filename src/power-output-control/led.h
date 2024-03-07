#ifndef LED_H_
#define LED_H_

#include "stdbool.h"

void led_init();
void led_set_overloaded(bool overload);
void led_set_poweroutput(bool overload);

#endif /* #define LED_H_ */
