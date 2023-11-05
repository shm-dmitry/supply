#ifndef USER_INPUT_H_
#define USER_INPUT_H_

#include "stdint.h"

#define USER_INPUT_STATUS_NONE       0x00
#define USER_INPUT_STATUS_LEFT       0x01
#define USER_INPUT_STATUS_RIGHT      0x02
#define USER_INPUT_STATUS_LEFT_FAST  0x03
#define USER_INPUT_STATUS_RIGHT_FAST 0x04
#define USER_INPUT_STATUS_CLICK      0x05

#define USER_INPUT_IS_FAST(x) ((x) == USER_INPUT_STATUS_LEFT_FAST || (x) == USER_INPUT_STATUS_RIGHT_FAST)

void user_input_init();

uint8_t user_input_encoder_i_status();
uint8_t user_input_encoder_v_status();
bool user_input_onoff_pressed();
void user_input_start();

#endif /* #define USER_INPUT_H_ */
