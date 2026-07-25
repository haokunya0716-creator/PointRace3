
#ifndef LINE_DETECT_H_
#define LINE_DETECT_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

extern volatile uint8_t line_detect[8];//黑线为1，白线为0

/**
 * @brief 扫描 8 路灰度循迹传感器并更新黑白检测结果。
 * @property 黑线低电平有效，line_detect[i] 为 1 表示黑线，为 0 表示白线。
 */
void Line_detect(void);

/**
 * @brief 初始化循迹 PID 控制器。
 * @property 循迹位置使用 0~7 权重，目标中心值为 3.5，PID 输出限幅为 -60~60。
 */
void App_Line_Follow_Init(void);

/**
 * @brief 复位循迹 PID 控制器和循迹位置缓存。
 * @property 复位后默认循迹位置回到中心值 3.5。
 */
void App_Line_Follow_Reset(void);

/**
 * @brief 刷新灰度检测并计算当前黑线位置平均值。
 * @property 每路权重为数组下标 0~7；未检测到黑线时沿用 last_line_value，避免除以 0。
 */
void Line_Follow_UpdateData(void);

/**
 * @brief 执行一次循迹控制进程。
 * @property 根据 line_value 计算左右轮速度差，并通过 App_Speed_Set 输出目标速度。
 */
void App_Line_Follow_Proc(void);

#endif /* LINE_DETECT_H_ */
