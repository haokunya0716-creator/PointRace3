#ifndef APP_TASK4_H_
#define APP_TASK4_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

// 投放标志位：0 不投放，1/2/3 分别投放 1/2/3 号舵机。
// 由视觉识别等其他任务写入，Task4 在到达格子中心时读取并处理。
extern volatile uint8_t drop_flag;

void Task4(uint8_t exit);

#endif
