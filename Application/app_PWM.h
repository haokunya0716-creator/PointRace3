#ifndef APP_PWM_H_
#define APP_PWM_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define APP_PWM_SERVO_MIN_US   500U    // 0 度对应的高电平时间，单位 us
#define APP_PWM_SERVO_MAX_US   2500U   // 180 度对应的高电平时间，单位 us
#define APP_PWM_SERVO_MAX_DEG  180.0f  // 180 度舵机的最大角度
#define APP_PWM_TICK_US        2U      // SysConfig 中 PWM 计数频率为 500kHz，所以 1 个计数 = 2us

/**
 * @brief 舵机 PWM 输出通道编号。
 * @note 名字按照原理图连接器编号命名，方便和 PCB 上的 CN6/CN7/CN12/CN13 对照。
 */
typedef enum {
    APP_PWM_CN6 = 0,  // CN6：TIMA1_CH0
    APP_PWM_CN7,      // CN7：TIMA1_CH1
    APP_PWM_CN12,     // CN12：TIMG6_CH0
    APP_PWM_CN13,     // CN13：TIMG6_CH1
    APP_PWM_NUM       // PWM 舵机通道数量
} AppPWM_Id_t;

/**
 * @brief 初始化舵机 PWM。
 * @param 无
 * @return 无
 * @note SysConfig 已经配置好定时器和引脚，这里只把四路舵机置中到 90 度并启动定时器。
 */
void App_PWM_Init(void);

/**
 * @brief 设置某一路 180 度舵机的目标角度。
 * @param id 舵机通道，可选 APP_PWM_CN6、APP_PWM_CN7、APP_PWM_CN12、APP_PWM_CN13。
 * @param deg 目标角度，范围 0~180 度；超出范围会自动限幅。
 * @return 无
 */
void App_PWM_SetAngle(AppPWM_Id_t id, float deg);

#endif /* APP_PWM_H_ */
