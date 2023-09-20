#ifndef GUI_IPAGECALLBACK_H_
#define GUI_IPAGECALLBACK_H_

#include "stdint.h"
#include "user_input.h"

#define IPAGECALLBACK_INPUT_TYPE_I 0x01
#define IPAGECALLBACK_INPUT_TYPE_V 0x02

typedef void (*t_page_callback)(uint8_t input_type, uint8_t userinput_status);

#endif /* GUI_IPAGECALLBACK_H_ */

