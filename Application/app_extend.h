#ifndef APP_EXTEND_H_
#define APP_EXTEND_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define EXT_ROUTE_LEN  8 // 每种小动物最多 8 个动作

typedef enum {
    EXT_IDLE = 0, // 未启动
    EXT_WAIT,     // 等待视觉识别
    EXT_RUN,      // 正在执行动作
    EXT_DONE      // 完成
} ExtendState_t;

extern volatile ExtendState_t extend_state; // 发挥任务状态
extern volatile uint8_t animal_id;          // 视觉识别到的小动物编号，0 表示未识别

/**
 * @brief 初始化发挥任务。
 * @param 无
 * @return 无
 */
void App_Extend_Init(void);

/**
 * @brief 启动发挥任务。
 * @param 无
 * @return 无
 */
void App_Extend_Start(void);

/**
 * @brief 发挥任务进程函数。
 * @param 无
 * @return 无
 */
void App_Extend_Proc(void);

#endif /* APP_EXTEND_H_ */
