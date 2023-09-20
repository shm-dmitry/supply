#ifndef GUI_MAIN_PAGE_H_
#define GUI_MAIN_PAGE_H_

#include "stdint.h"

void gui_main_page_on_refresh(uint8_t, uint8_t);
void gui_main_page_on_reset(uint8_t, uint8_t);
void gui_main_page_on_action(uint8_t input_type, uint8_t userinput_status);

#endif /* GUI_MAIN_PAGE_H_ */
