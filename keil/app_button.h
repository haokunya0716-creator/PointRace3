/*
* app_button.h
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */

#ifndef INC_APP_BUTTON_H_
#define INC_APP_BUTTON_H_


#include "ti_msp_dl_config.h"

void App_Button_Init(void);
void App_Button_Proc(void);

extern uint8_t task1_flag;
extern uint8_t task2_flag;
extern uint8_t task3_flag;
extern uint8_t task4_flag;
#endif /* INC_APP_BUTTON_H_ */
