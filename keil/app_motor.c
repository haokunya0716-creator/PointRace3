#include "app_motor.h"
#include "pid.h"
#include "imu.h"
#include "clock.h"
#include "motor_read_enc.h"
#include <math.h>

//减速比 28
//轮胎转一圈，磁圈转28圈，共有13个对极，四倍频计数,所以磁圈转一圈 encoder 加 28*13*4.0f
//最终得到的是转了多少度（一圈是三百六十度）
#define MOTOR_WHEEL    65.0f   // 轮子直径，单位 mm；实测不准时优先改这里
#define MOTOR_ENCODER_PPR     13.0f   // MG513 霍尔编码器线数
#define MOTOR_REDUCTION       28.0f   // MG513 减速比 1:28    reduction 减速
#define MOTOR_QUAD            4.0f    // 四倍频计数

#define PI              3.1415926f

#define MOTOR_MAX_SPEED       450.0f  // 电机板速度限幅
#define MOTOR_POS_ERROR_MAX    10.0f   // 位置误差小于 10mm 认为到位
#define MOTOR_ANGLE_ERROR_MAX  2.0f    // 角度误差小于 2 度认为到位

extern int16_t MA_Speed;
extern int16_t MB_Speed;
extern int16_t MC_Speed;
extern int16_t MD_Speed;

volatile uint8_t motor_pos_done = 1;   // 位置闭环完成标志
volatile uint8_t motor_angle_done = 1; // 角度闭环完成标志

static PID_TypeDef pos_pid;      // 位置闭环 PID，反馈量为 mm，输出量为速度指令
static PID_TypeDef angle_pid;    // 角度闭环 PID，反馈量为 deg，输出量为左右差速


static int16_t enc_start[4];     // 开始直行时四个编码器的初值

static float pos_target_mm = 0;  // 直行目标距离，单位 mm
static float angle_target = 0;   // 目标航向角，单位 deg

static float base_speed = 0;     // 位置闭环输出的基础前进速度

static uint8_t pos_en = 0;       // 1：位置闭环正在工作
static uint8_t angle_en = 0;     // 1：角度闭环正在工作

/**
 * @brief 限制速度指令范围。
 * @param speed 原始速度指令。
 * @return 限幅后的速度指令。
 */
static float LimitSpeed(float speed)
{
    if (speed > MOTOR_MAX_SPEED)
        speed = MOTOR_MAX_SPEED;
    if (speed < -MOTOR_MAX_SPEED)
        speed = -MOTOR_MAX_SPEED;
    return speed;
}

/**
 * @brief 把左右速度写入四个电机速度变量。
 * @param left 左侧速度。
 * @param right 右侧速度。
 * @return 无
 * @note 当前默认 A/C 为左侧，B/D 为右侧；如果实车方向不对，只改这里。
 */
static void Set_Motor_Speed(float left, float right)
{
    int16_t l = (int16_t)LimitSpeed(left);
    int16_t r = (int16_t)LimitSpeed(right);

    MA_Speed = l;
    MB_Speed = r;
    MC_Speed = l;
    MD_Speed = r;
}

/**
 * @brief 计算角度误差，自动处理 -180 到 180 度跳变。
 * @param target 目标角度。
 * @param current 当前角度。
 * @return 最短方向角度误差。
 */
static float Angle_Err_Compute(float target, float current)
{
    float err = target - current;

    while (err > 180.0f)
        err -= 360.0f;
    while (err < -180.0f)
        err += 360.0f;

    return err;
}

/**
 * @brief 获取本次直行已经走过的距离。
 * @param 无
 * @return 距离，单位 mm。
 */
static float Get_Pos(void)
{
    float enc_avg = 0;
    float count_per_rev = MOTOR_ENCODER_PPR * MOTOR_REDUCTION * MOTOR_QUAD;
    float wheel_mm = MOTOR_WHEEL * PI;

    for (uint8_t i = 0; i < 4; i++)
    {
        enc_avg += (float)((int16_t)(modbus_date[i] - enc_start[i]));
    }
    enc_avg *= 0.25f;

    return enc_avg * wheel_mm / count_per_rev;
}

/**
 * @brief 角度 PID 计算。
 * @param pid PID 控制器。
 * @param yaw 当前 yaw 角。
 * @return 左右差速修正量。
 */
static float PID_Angle_Compute(PID_TypeDef *pid, float yaw)
{
    float err = Angle_Err_Compute(pid->SP, yaw);

    unsigned long now_ms = 0;
    mspm0_get_clock_ms(&now_ms);
    uint64_t t_k = now_ms;

    unsigned long delta_ms = now_ms - (unsigned long)pid->t_k_1;
    float deltaT = delta_ms * 1.0e-3f;
    if (deltaT <= 0.0f)
        deltaT = 1.0e-3f;

    float err_dev = 0.0f;
    float err_int = 0.0f;

    if (pid->t_k_1 != 0)
    {
        err_dev = (err - pid->err_k_1) / deltaT;
        err_int = pid->err_int_k_1 + (err + pid->err_k_1) * deltaT * 0.5f;
    }

    float out = pid->Kp * err + pid->Ki * err_int + pid->Kd * err_dev;

    pid->t_k_1 = t_k;
    pid->err_k_1 = err;
    pid->err_int_k_1 = err_int;

    if (out > pid->UpperLimit)
        out = pid->UpperLimit;
    if (out < pid->LowerLimit)
        out = pid->LowerLimit;

    return out;
}

/**
 * @brief 初始化位置闭环和角度闭环 PID。
 * @param 无
 * @return 无
 */
void App_Motor_Init(void)
{
    PID_Init(&pos_pid, 1.2f, 0.0f, 0.02f);
    PID_LimitConfig(&pos_pid, MOTOR_MAX_SPEED, -MOTOR_MAX_SPEED);

    PID_Init(&angle_pid, 3.0f, 0.0f, 0.06f);
    PID_LimitConfig(&angle_pid, 260.0f, -260.0f);

    Set_Motor_Speed(0, 0);
}

/**
 * @brief 设置直行距离目标。
 * @param distance_mm 目标距离，单位 mm。
 * @return 无
 */
void App_Motor_SetPosition(float distance_mm)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        enc_start[i] = modbus_date[i];
    }

    pos_target_mm = distance_mm;
    PID_Reset(&pos_pid);
    PID_ChangeSP(&pos_pid, pos_target_mm);

    angle_target = getYaw();
    PID_Reset(&angle_pid);
    PID_ChangeSP(&angle_pid, angle_target);

    base_speed = 0;
    pos_en = 1;
    angle_en = 1;
    motor_pos_done = 0;
    motor_angle_done = 0;
}

/**
 * @brief 设置相对转角目标。
 * @param delta_deg 相对当前角度要转多少度。
 * @return 无
 */
void App_Motor_SetTurn(float delta_deg)
{
    pos_en = 0;
    base_speed = 0;

    angle_target = getYaw() + delta_deg;
    PID_Reset(&angle_pid);
    PID_ChangeSP(&angle_pid, angle_target);

    angle_en = 1;
    motor_angle_done = 0;
}

/**
 * @brief 位置闭环进程函数。
 * @param 无
 * @return 无
 */
void App_Motor_Position_Proc(void)
{
    if (!pos_en)
        return;

    float pos_now = Get_Pos();
    float err = pos_target_mm - pos_now;

    if (fabsf(err) < MOTOR_POS_ERROR_MAX)
    {
        pos_en = 0;
        angle_en = 0;
        base_speed = 0;
        motor_pos_done = 1;
        motor_angle_done = 1;
        Set_Motor_Speed(0, 0);
        return;
    }

    base_speed = PID_Compute(&pos_pid, pos_now);
}

/**
 * @brief 角度闭环进程函数。
 * @param 无
 * @return 无
 */
void App_Motor_Angle_Proc(void)
{
    float turn = 0;

    if (angle_en)
    {
        float yaw = getYaw();
        float err = Angle_Err_Compute(angle_target, yaw);

        if (!pos_en && fabsf(err) < MOTOR_ANGLE_ERROR_MAX)
        {
            angle_en = 0;
            motor_angle_done = 1;
            Set_Motor_Speed(0, 0);
            return;
        }

        turn = PID_Angle_Compute(&angle_pid, yaw);
    }

    Set_Motor_Speed(base_speed - turn, base_speed + turn);
}
