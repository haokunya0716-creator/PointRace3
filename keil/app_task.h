#ifndef APP_TASK_H_
#define APP_TASK_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

void Task1(void);
void Task2(void);
//void Task3(void);
void Task4_SetExitByNumber(uint8_t number);
void Task4(void);

#endif
