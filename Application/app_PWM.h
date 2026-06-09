#ifndef APP_PWM_H_
#define APP_PWM_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define APP_PWM_SERVO_MIN_US      500U
#define APP_PWM_SERVO_MAX_US      2500U
#define APP_PWM_SERVO_PERIOD_CNT  10000U
#define APP_PWM_SERVO_MAX_DEG     180.0f
#define APP_PWM_TICK_US           2U

typedef enum {
    APP_PWM_SERVO1 = 0,
    APP_PWM_SERVO2,
    APP_PWM_SERVO3,
    APP_PWM_NUM,

    APP_PWM_CN6 = APP_PWM_SERVO1,
    APP_PWM_CN7 = APP_PWM_SERVO2,
    APP_PWM_CN11 = APP_PWM_SERVO3
} AppPWM_Id_t;

void App_PWM_Init(void);
void App_PWM_SetAngle(AppPWM_Id_t id, float deg);
float App_PWM_GetAngle(AppPWM_Id_t id);

#endif /* APP_PWM_H_ */
