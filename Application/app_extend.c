#include "app_extend.h"
#include "../keil/app_motor.h"

#define ANIMAL_NUM  4 // 小动物编号范围：1~3，0 表示未识别

volatile ExtendState_t extend_state = EXT_IDLE; // 发挥任务状态
volatile uint8_t animal_id = 0;                 // 当前识别到的小动物编号

static uint8_t route_i = 0; // 当前执行到第几个动作

// turn_route：转角动作，单位 deg；dist_route：直行动作，单位 mm。
// 视觉和路线还没定时，先全部留空。后面只需要按小动物编号填写这三张表。
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

/**
 * @brief 读取视觉识别结果。
 * @param 无
 * @return 0：未识别；1~3：小动物编号。
 * @note 视觉接口先空着，后面接视觉协议时只改这个函数。
 */
static uint8_t Vision_GetAnimal(void)
{
    return 0;
}

/**
 * @brief 执行当前动作。
 * @param 无
 * @return 无
 */
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

/**
 * @brief 初始化发挥任务。
 * @param 无
 * @return 无
 */
void App_Extend_Init(void)
{
    extend_state = EXT_IDLE;
    animal_id = 0;
    route_i = 0;
}

/**
 * @brief 启动发挥任务。
 * @param 无
 * @return 无
 */
void App_Extend_Start(void)
{
    extend_state = EXT_WAIT;
    animal_id = 0;
    route_i = 0;
}

/**
 * @brief 发挥任务进程函数。
 * @param 无
 * @return 无
 */
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
