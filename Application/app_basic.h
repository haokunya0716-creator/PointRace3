#ifndef APP_BASIC_H_
#define APP_BASIC_H_

#include <stdint.h>

#define BASIC_GRID_MM        500.0f // 一格距离，单位 mm
#define BASIC_MAX_STEP       16     // 基础部分最多走 16 格
#define BASIC_LINE_COUNT     4      // 灰度检测到 4 路及以上黑线，认为有压线风险

typedef enum {
    BASIC_IDLE = 0, // 未启动
    BASIC_CHECK,    // 判断前方能不能走
    BASIC_GO,       // 正在走一格
    BASIC_TURN,     // 正在转向
    BASIC_DONE      // 结束
} BasicState_t;

extern volatile BasicState_t basic_state; // 基础任务状态
extern volatile uint8_t basic_step;       // 已经走完的格子数

/**
 * @brief 初始化基础任务。
 * @param 无
 * @return 无
 */
void App_Basic_Init(void);

/**
 * @brief 启动基础任务。
 * @param 无
 * @return 无
 */
void App_Basic_Start(void);

/**
 * @brief 基础任务进程函数。
 * @param 无
 * @return 无
 */
void App_Basic_Proc(void);

#endif /* APP_BASIC_H_ */
