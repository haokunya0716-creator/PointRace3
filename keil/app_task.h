#ifndef APP_TASK_H_
#define APP_TASK_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

void Task1(void);
void Task2(void);


extern uint8_t task_exit_x;
extern uint8_t task_exit_y;

#endif
