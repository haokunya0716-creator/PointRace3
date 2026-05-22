/*
 * task.h
 *
 *  Created on: May 5, 2025
 *      Author: gaoxi
 */

#ifndef INC_TASK_H_
#define INC_TASK_H_

#include "clock.h"  // 引入 SysTick 全局计时变量 tick_ms

// 周期任务调度宏定义 (基于 SysTick)
#define PERIODIC(T) \
static unsigned long nxt = 0; \
if(tick_ms < nxt) return; \
nxt += (T);

#define PERIODIC_START(NAME, T) \
static unsigned long NAME##_nxt = 0; \
if(tick_ms >= NAME##_nxt) {\
NAME##_nxt += (T);

#define PERIODIC_END }

#endif /* INC_TASK_H_ */