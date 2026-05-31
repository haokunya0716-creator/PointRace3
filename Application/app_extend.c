#include "app_extend.h"
#include "../keil/app_motor.h"

#define ANIMAL_NUM  4

volatile ExtendState_t extend_state = EXT_IDLE;
volatile uint8_t animal_id = 0;
static uint8_t route_i = 0;

static const int16_t turn_route[ANIMAL_NUM][EXT_ROUTE_LEN] = {
    {0},
    {0},
    {0},
    {0},
};

static const float dist_route[ANIMAL_NUM][EXT_ROUTE_LEN] = {
    {0},
    {0},
    {0},
    {0},
};

static const uint8_t route_len[ANIMAL_NUM] = {
    0,
    0,
    0,
    0,
};

static uint8_t Vision_GetAnimal(void)
{
    return 0;
}

static void Run_One(void)
{
    if (route_i >= route_len[animal_id])
    {
        extend_state = EXT_DONE;
        return;
    }

    if (turn_route[animal_id][route_i] != 0)
        App_Motor_SetTurn((float)turn_route[animal_id][route_i]);
    else
        App_Motor_SetPosition(dist_route[animal_id][route_i]);
}

void App_Extend_Init(void)
{
    extend_state = EXT_IDLE;
    animal_id = 0;
    route_i = 0;
}

void App_Extend_Start(void)
{
    extend_state = EXT_WAIT;
    animal_id = 0;
    route_i = 0;
}

void App_Extend_Proc(void)
{
    if (extend_state == EXT_IDLE || extend_state == EXT_DONE)
        return;

    if (extend_state == EXT_WAIT)
    {
        animal_id = Vision_GetAnimal();

        if (animal_id > 0 && animal_id < ANIMAL_NUM)
        {
            route_i = 0;
            Run_One();
            if (extend_state != EXT_DONE)
                extend_state = EXT_RUN;
        }
        return;
    }

    if (extend_state == EXT_RUN && motor_pos_done && motor_angle_done)
    {
        route_i++;
        Run_One();
    }
}
