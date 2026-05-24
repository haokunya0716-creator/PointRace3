#ifndef APP_MOTOR_H_
#define APP_MOTOR_H_

#include <stdint.h>

extern volatile uint8_t motor_pos_done;   // 1：位置闭环已到达目标
extern volatile uint8_t motor_angle_done; // 1：角度闭环已到达目标

/**
 * @brief 初始化位置闭环和角度闭环 PID。
 * @param 无
 * @return 无
 */
void App_Motor_Init(void);

/**
 * @brief 设置直行距离目标。
 * @param distance_mm 目标距离，单位 mm；正数前进，负数后退。
 * @return 无
 */
void App_Motor_SetPosition(float distance_mm);

/**
 * @brief 设置相对转角目标。
 * @param delta_deg 相对当前角度要转多少度；正负方向按实车测试确认。
 * @return 无
 */
void App_Motor_SetTurn(float delta_deg);

/**
 * @brief 位置闭环进程函数。
 * @param 无
 * @return 无
 * @note 建议在 empty.c 的 10ms 周期任务中调用。
 */
void App_Motor_Position_Proc(void);

/**
 * @brief 角度闭环进程函数。
 * @param 无
 * @return 无
 * @note 建议在 empty.c 的 10ms 周期任务中调用，且放在 App_Motor_Position_Proc() 后面。
 */
void App_Motor_Angle_Proc(void);

#endif /* APP_MOTOR_H_ */
