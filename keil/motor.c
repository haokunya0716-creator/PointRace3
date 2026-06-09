#include "motor.h"

#define MOTOR_PWM_PERIOD_COUNT 8000U

/* MOTOR: TIMA0
 * Left motor:  AIN2 -> PB8  -> C0, AIN1 -> PB9  -> C1
 * Right motor: BIN2 -> PB12 -> C2, BIN1 -> PB13 -> C3
 */
#define MOTOR_AIN2_CC GPIO_MOTOR_C0_IDX
#define MOTOR_AIN1_CC GPIO_MOTOR_C1_IDX
#define MOTOR_BIN2_CC GPIO_MOTOR_C2_IDX
#define MOTOR_BIN1_CC GPIO_MOTOR_C3_IDX

static uint8_t motor_enabled = 0;

static float Motor_LimitDuty(float duty)
{
    if (duty > 100.0f)
        duty = 100.0f;
    if (duty < -100.0f)
        duty = -100.0f;

    return duty;
}

static uint32_t Motor_DutyToCompare(float duty)
{
    uint32_t active_count = 0;

    if (duty < 0.0f)
        duty = -duty;

    if (duty > 100.0f)
        duty = 100.0f;

    active_count = (uint32_t)(duty / 100.0f * (float)MOTOR_PWM_PERIOD_COUNT + 0.5f);
    if (active_count > MOTOR_PWM_PERIOD_COUNT)
        active_count = MOTOR_PWM_PERIOD_COUNT;

    return MOTOR_PWM_PERIOD_COUNT - active_count;
}

static void Motor_SetChannel(DL_TIMER_CC_INDEX cc, uint32_t compare)
{
    if (compare > MOTOR_PWM_PERIOD_COUNT)
        compare = MOTOR_PWM_PERIOD_COUNT;

    DL_Timer_setCaptureCompareValue(MOTOR_INST, compare, cc);
}

void Motor_Stop_L(void)
{
    Motor_SetChannel(MOTOR_AIN1_CC, MOTOR_PWM_PERIOD_COUNT);
    Motor_SetChannel(MOTOR_AIN2_CC, MOTOR_PWM_PERIOD_COUNT);
}

void Motor_Stop_R(void)
{
    Motor_SetChannel(MOTOR_BIN1_CC, MOTOR_PWM_PERIOD_COUNT);
    Motor_SetChannel(MOTOR_BIN2_CC, MOTOR_PWM_PERIOD_COUNT);
}

//
// @brief: Init AT8236 motor PWM.
//
void Motor_Init(void)
{
    motor_enabled = 1;

    Motor_Stop_L();
    Motor_Stop_R();

    DL_Timer_startCounter(MOTOR_INST);
	
		Motor_Cmd(1);
	
}

void Motor_Stop(void){
	
	Motor_Stop_L();
	Motor_Stop_R();
	
}

//
// @brief: Enable or disable AT8236 output.
// @param on 0 - stop output, nonzero - allow Motor_Set_L/R output.
//
void Motor_Cmd(uint8_t on)
{
    motor_enabled = (on != 0U) ? 1U : 0U;

    if (motor_enabled == 0U)
    {
        Motor_Stop_L();
        Motor_Stop_R();
    }
}

//
// @brief: Set left motor duty, range -100.0f ~ +100.0f.
// @note: Fast decay mode. Forward: AIN1=PWM/AIN2=0. Reverse: AIN1=0/AIN2=PWM.
//
void Motor_Set_L(float duty)
{
    uint32_t compare = 0;

    if (motor_enabled == 0U)
    {
        Motor_Stop_L();
        return;
    }

    duty = Motor_LimitDuty(-duty);
    compare = Motor_DutyToCompare(duty);

    if (duty > 0.0f)
    {
        Motor_SetChannel(MOTOR_AIN2_CC, MOTOR_PWM_PERIOD_COUNT);
        Motor_SetChannel(MOTOR_AIN1_CC, compare);
    }
    else if (duty < 0.0f)
    {
        Motor_SetChannel(MOTOR_AIN1_CC, MOTOR_PWM_PERIOD_COUNT);
        Motor_SetChannel(MOTOR_AIN2_CC, compare);
    }
    else
    {
        Motor_Stop_L();
    }
}

//
// @brief: Set right motor duty, range -100.0f ~ +100.0f.
// @note: Fast decay mode. Forward: BIN1=PWM/BIN2=0. Reverse: BIN1=0/BIN2=PWM.
//
void Motor_Set_R(float duty)
{
    uint32_t compare = 0;

    if (motor_enabled == 0U)
    {
        Motor_Stop_R();
        return;
    }

    duty = Motor_LimitDuty(duty);
    compare = Motor_DutyToCompare(duty);

    if (duty > 0.0f)
    {
        Motor_SetChannel(MOTOR_BIN2_CC, MOTOR_PWM_PERIOD_COUNT);
        Motor_SetChannel(MOTOR_BIN1_CC, compare);
    }
    else if (duty < 0.0f)
    {
        Motor_SetChannel(MOTOR_BIN1_CC, MOTOR_PWM_PERIOD_COUNT);
        Motor_SetChannel(MOTOR_BIN2_CC, compare);
    }
    else
    {
        Motor_Stop_R();
    }
}
