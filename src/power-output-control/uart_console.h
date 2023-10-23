#ifndef UART_CONSOLEH_
#define UART_CONSOLEH_

#include "config.h"

#if UART_ENABLED
void uart_console_init();
void uart_console_on_main_loop();
#endif

#endif /* #define UART_CONSOLEH_ */
