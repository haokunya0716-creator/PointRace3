#include "app_PWM.h"

typedef struct {
//    GPTIMER_Regs *timer;
//    DL_TIMER_CC_INDEX cc;
//    uint32_t period_cnt;
//    uint32_t tick_us;
} PwmCh_t;

static const PwmCh_t pwm_ch[APP_PWM_NUM] = {
//    [APP_PWM_SERVO1] = {SERVO_INST, GPIO_SERVO_C0_IDX, APP_PWM_SERVO_PERIOD_CNT, 2U},
//    [APP_PWM_SERVO2] = {SERVO_INST, GPIO_SERVO_C1_IDX, APP_PWM_SERVO_PERIOD_CNT, 2U},
//    [APP_PWM_SERVO3] = {SERVO3_INST, GPIO_SERVO3_C1_IDX, APP_PWM_SERVO_PERIOD_CNT, 2U},
};

static float pwm_angle[APP_PWM_NUM] = {0};

static float ClampAngle(float deg)
{
    if (deg < 0.0f)
        deg = 0.0f;
    if (deg > APP_PWM_SERVO_MAX_DEG)
        deg = APP_PWM_SERVO_MAX_DEG;

    return deg;
}

//static uint32_t AngleToCmp(AppPWM_Id_t id, float deg)
//{
////    uint32_t pulse_cnt = 0;
////    uint32_t pulse_us = 0;

////    deg = ClampAngle(deg);
////    pulse_us = (uint32_t)(APP_PWM_SERVO_MIN_US +
////        deg * (float)(APP_PWM_SERVO_MAX_US - APP_PWM_SERVO_MIN_US) / APP_PWM_SERVO_MAX_DEG);
////    pulse_cnt = pulse_us / pwm_ch[id].tick_us;

////    if (pulse_cnt > pwm_ch[id].period_cnt)
////        pulse_cnt = pwm_ch[id].period_cnt;

////    return pulse_cnt;
//}

void App_PWM_Init(void)
{
//    App_PWM_SetAngle(APP_PWM_SERVO1, 180.0f);
//    App_PWM_SetAngle(APP_PWM_SERVO2, 110.0f);
//    App_PWM_SetAngle(APP_PWM_SERVO3, 125.0f);

//    DL_Timer_startCounter(SERVO_INST);
//    DL_Timer_startCounter(SERVO3_INST);
}

void App_PWM_SetAngle(AppPWM_Id_t id, float deg)
{
//    if (id >= APP_PWM_NUM)
//        return;

//    deg = ClampAngle(deg);
//    pwm_angle[id] = deg;
//    DL_Timer_setCaptureCompareValue(pwm_ch[id].timer, AngleToCmp(id, deg), pwm_ch[id].cc);
}

float App_PWM_GetAngle(AppPWM_Id_t id)
{
//    if (id >= APP_PWM_NUM)
//        return 0.0f;

//    return pwm_angle[id];
	return 0.0f;
}
