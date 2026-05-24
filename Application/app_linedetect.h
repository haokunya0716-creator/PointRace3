#ifndef APP_LINEDETECT_H_
#define APP_LINEDETECT_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define LINE_SENSOR_NUM           8 // 八路灰度传感器通道数量，X1~X8
#define LINE_SENSOR_ACTIVE_LEVEL  1 // 黑线有效电平；实测如果黑线读到 0，就把这里改成 0

extern volatile uint8_t line_raw[LINE_SENSOR_NUM]; // X1~X8 原始读数，每个元素为 0 或 1
extern volatile uint8_t line_black_flag;           // 1：至少一路检测到黑线；0：没有检测到黑线
extern volatile uint8_t line_cross_flag;           // 1：多路同时检测到黑线，可用于横线/边界线判断
extern volatile uint8_t line_black_count;          // 当前检测到黑线的通道数量，范围 0~8
extern volatile uint8_t line_black_mask;           // 黑线检测位图，bit0=X1，bit7=X8

/**
 * @brief 初始化八路灰度传感器 GPIO。
 * @param 无
 * @return 无
 * @note SysConfig 已经生成 LINE_FOLLOW_xxx 宏；这里直接使用，不再重复定义引脚。
 */
void App_LineDetect_Init(void);

/**
 * @brief 扫描八路灰度传感器，并更新黑线检测状态。
 * @param 无
 * @return 无
 * @note 本函数只检测黑线/边界，不做循迹 PID，也不控制电机。
 */
void App_LineDetect_Proc(void);

#endif /* APP_LINEDETECT_H_ */
