#include "app_PWM.h"

/**
 * @brief 单路 PWM 的定时器和比较通道信息。
 * @note 这是模块内部用的映射表，不放到 h 文件里，避免外部代码依赖这些底层细节。
 */
typedef struct {
    GPTIMER_Regs *timer;  // SysConfig 生成的定时器实例
    DL_TIMER_CC_INDEX cc; // SysConfig 生成的比较通道
} PwmCh_t;

/**
 * @brief 四路舵机 PWM 通道映射表。
 * @note 数组下标使用 AppPWM_Id_t；顺序和原理图连接器编号对应。
 */
static const PwmCh_t pwm_ch[APP_PWM_NUM] = {
    [APP_PWM_CN6]  = {SERVO_1_INST, GPIO_SERVO_1_C0_IDX},
    [APP_PWM_CN7]  = {SERVO_1_INST, GPIO_SERVO_1_C1_IDX},
    [APP_PWM_CN12] = {PWM_0_INST,   GPIO_PWM_0_C0_IDX},
    [APP_PWM_CN13] = {PWM_0_INST,   GPIO_PWM_0_C1_IDX},
};

/**
 * @brief 把舵机角度换算成定时器比较值。
 * @param deg 目标角度，单位度。
 * @return 定时器比较值。
 * @note 180 度舵机通常是 0.5ms=0度、1.5ms=90度、2.5ms=180度。
 */
static uint32_t AngleToCmp(float deg)
{
    if (deg < 0.0f)
        deg = 0.0f;
    if (deg > APP_PWM_SERVO_MAX_DEG)
        deg = APP_PWM_SERVO_MAX_DEG;

    uint16_t us = (uint16_t)(APP_PWM_SERVO_MIN_US +
        deg * (APP_PWM_SERVO_MAX_US - APP_PWM_SERVO_MIN_US) / APP_PWM_SERVO_MAX_DEG);

    return (uint32_t)(us / APP_PWM_TICK_US);
}

/**
 * @brief 初始化舵机 PWM。
 * @param 无
 * @return 无
 */
void App_PWM_Init(void)
{
    uint8_t i = 0; // 当前初始化的舵机通道下标

    for (i = 0; i < APP_PWM_NUM; i++)
    {
        App_PWM_SetAngle((AppPWM_Id_t)i, 90.0f);
    }

    DL_Timer_startCounter(SERVO_1_INST);
    DL_Timer_startCounter(PWM_0_INST);
}

/**
 * @brief 设置某一路 180 度舵机的目标角度。
 * @param id 舵机通道编号。
 * @param deg 目标角度，单位度。
 * @return 无
 */
void App_PWM_SetAngle(AppPWM_Id_t id, float deg)
{
    if (id >= APP_PWM_NUM)
        return;

    DL_Timer_setCaptureCompareValue(pwm_ch[id].timer, AngleToCmp(deg), pwm_ch[id].cc);
}
