#include "app_basic.h"
#include "app_linedetect.h"
#include "app_vl5310x.h"
#include "../keil/app_motor.h"

#define TURN_LEFT_DEG   (-90.0f)
#define TURN_RIGHT_DEG  (90.0f)

extern int16_t MA_Speed;
extern int16_t MB_Speed;
extern int16_t MC_Speed;
extern int16_t MD_Speed;

volatile BasicState_t basic_state = BASIC_IDLE;
volatile uint8_t basic_step = 0;

static void Car_Stop(void)
{
    App_Motor_SetPosition(0.0f);
    MA_Speed = 0;
    MB_Speed = 0;
    MC_Speed = 0;
    MD_Speed = 0;
}

static float Choose_Turn(void)
{
    if (vl5310x_left_obstacle_flag && !vl5310x_right_obstacle_flag)
        return TURN_RIGHT_DEG;

    if (vl5310x_right_obstacle_flag && !vl5310x_left_obstacle_flag)
        return TURN_LEFT_DEG;

    if (App_VL5310X_GetDistance(VL5310X_LEFT) > App_VL5310X_GetDistance(VL5310X_RIGHT))
        return TURN_LEFT_DEG;

    return TURN_RIGHT_DEG;
}

void App_Basic_Init(void)
{
    basic_state = BASIC_IDLE;
    basic_step = 0;
}

void App_Basic_Start(void)
{
    basic_state = BASIC_CHECK;
    basic_step = 0;
}

void App_Basic_Proc(void)
{
    if (basic_state == BASIC_IDLE || basic_state == BASIC_DONE)
        return;

    if (line_black_count >= BASIC_LINE_COUNT)
    {
        Car_Stop();
        basic_state = BASIC_DONE;
        return;
    }

    switch (basic_state)
    {
        case BASIC_CHECK:
            if (basic_step >= BASIC_MAX_STEP)
            {
                Car_Stop();
                basic_state = BASIC_DONE;
            }
            else if (vl5310x_front_obstacle_flag)
            {
                App_Motor_SetTurn(Choose_Turn());
                basic_state = BASIC_TURN;
            }
            else
            {
                App_Motor_SetPosition(BASIC_GRID_MM);
                basic_state = BASIC_GO;
            }
            break;

        case BASIC_GO:
            if (vl5310x_front_obstacle_flag)
            {
                Car_Stop();
                basic_state = BASIC_CHECK;
            }
            else if (motor_pos_done)
            {
                basic_step++;
                basic_state = BASIC_CHECK;
            }
            break;

        case BASIC_TURN:
            if (motor_angle_done)
                basic_state = BASIC_CHECK;
            break;

        default:
            break;
    }
}
