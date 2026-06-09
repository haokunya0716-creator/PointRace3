#include "app_motor.h"
#include "clock.h"
#include "imu.h"
#include "task.h"
#include "app_encoder.h"
#include "app_speed.h"
#include <math.h>

static PID_TypeDef pid_position_mid;
static PID_TypeDef pid_angle_yaw;

float motor_left_speed = 0.0f;
float motor_right_speed = 0.0f;

int16_t motor_encoder1 = 0;
int16_t motor_encoder2 = 0;

float position_l = 0.0f;
float position_r = 0.0f;
float position_mid = 0.0f;
float position_mid_ref = 0.0f;

float angle_yaw = 0.0f;
float angle_yaw_ref = 0.0f;

float motor_base_speed = 0.0f;
float motor_turn_speed = 0.0f;

volatile uint8_t motor_pos_done = 1;
volatile uint8_t motor_angle_done = 1;

static float App_Motor_Abs(float x)
{
    if(x < 0.0f){
        return -x;
    }

    return x;
}

static float App_Motor_AngleError(float target, float current)
{
    float err = target - current;

    while(err > 180.0f){
        err -= 360.0f;
    }

    while(err < -180.0f){
        err += 360.0f;
    }

    return err;
}

static float App_Motor_PositionSpeed(float err)
{
    float speed = 0.0f;
    float err_abs = App_Motor_Abs(err);

    // 距离较远时匀速走，快到目标时再按剩余距离减速。
    if(err_abs > 18.0f){
        speed = 40.0f;
    }else {
        speed = 40.0f * err_abs / 18.0f;

        if(speed < 20.0f){
            speed = 20.0f;
        }
    }

    if(err < 0.0f){
        speed = -speed;
    }

    return speed;
}

static float App_Motor_PIDComputeAngle(PID_TypeDef *PID, float FB)
{
    float err = App_Motor_AngleError(PID->SP, FB);
    unsigned long now_ms = 0;
    uint64_t t_k = 0;
    unsigned long delta_ms = 0;
    float deltaT = 0.0f;
    float err_dev = 0.0f;
    float err_int = 0.0f;
    float COp = 0.0f;
    float COi = 0.0f;
    float COd = 0.0f;
    float CO = 0.0f;

    mspm0_get_clock_ms(&now_ms);
    t_k = now_ms;
    delta_ms = now_ms - (unsigned long)PID->t_k_1;
    deltaT = delta_ms * 1.0e-3f;

    if(deltaT <= 0.0f){
        deltaT = 1.0e-3f;
    }

    if(PID->t_k_1 != 0){
        err_dev = (err - PID->err_k_1) / deltaT;

        if(fabsf(err) < 10.0f){
            err_int = PID->err_int_k_1 + (err + PID->err_k_1) * deltaT * 0.5f;
        }else {
            err_int = 0.0f;
        }
    }

    COp = PID->Kp * err;
    COi = PID->Ki * err_int;
    COd = PID->Kd * err_dev;
    CO = COp + COi + COd;

    PID->t_k_1 = t_k;
    PID->err_k_1 = err;
    PID->err_int_k_1 = err_int;

    if(CO > PID->UpperLimit){
        CO = PID->UpperLimit;
    }

    if(CO < PID->LowerLimit){
        CO = PID->LowerLimit;
    }

    if(PID->err_int_k_1 > PID->UpperLimit){
        PID->err_int_k_1 = PID->UpperLimit;
    }

    if(PID->err_int_k_1 < PID->LowerLimit){
        PID->err_int_k_1 = PID->LowerLimit;
    }

    return CO;
}

void App_Update_Data(void)
{
    angle_yaw = getYaw();

    motor_encoder1 = App_Encoder_GetEncoder_L();
    motor_encoder2 = App_Encoder_GetEncoder_R();

    position_l = App_Encoder_GetPos_L() / 360.0f * PI * MOTOR_WHEEL_DIA_CM;
    position_r = App_Encoder_GetPos_R() / 360.0f * PI * MOTOR_WHEEL_DIA_CM;
    position_mid = (position_l + position_r) * 0.5f;
}

void App_Motor_Init(void)
{
    App_Update_Data();

    PID_Init(&pid_position_mid, 1.5f, 0.0f, 0.0f);
    PID_LimitConfig(&pid_position_mid, 45.0f, -45.0f);

    PID_Init(&pid_angle_yaw, 0.6f, 0.0f, 0.0f);
    PID_LimitConfig(&pid_angle_yaw, 50.0f, -50.0f);

    position_mid_ref = position_mid;
    angle_yaw_ref = angle_yaw;

    motor_left_speed = 0.0f;
    motor_right_speed = 0.0f;
    motor_base_speed = 0.0f;
    motor_turn_speed = 0.0f;

    motor_pos_done = 1;
    motor_angle_done = 1;
}

void App_PID_Reset(void)
{
    PID_Reset(&pid_position_mid);
    PID_Reset(&pid_angle_yaw);
    App_Speed_Reset();
}

void App_Position_Angle_Reset(void)
{
    App_PID_Reset();

    motor_left_speed = 0.0f;
    motor_right_speed = 0.0f;
    motor_base_speed = 0.0f;
    motor_turn_speed = 0.0f;

    motor_pos_done = 1;
    motor_angle_done = 1;
}

void Set_Angle_SP(float angle_ref)
{
    App_Update_Data();

    angle_yaw_ref = angle_yaw + angle_ref;
    PID_ChangeSP(&pid_angle_yaw, angle_yaw_ref);
    PID_Reset(&pid_angle_yaw);

    motor_angle_done = 0;
}

void Set_Position_SP(float position_ref)
{
    App_Update_Data();

    position_mid_ref = position_mid + position_ref;
    angle_yaw_ref = angle_yaw;
    PID_ChangeSP(&pid_position_mid, position_mid_ref);
    PID_ChangeSP(&pid_angle_yaw, angle_yaw_ref);
    PID_Reset(&pid_position_mid);
    PID_Reset(&pid_angle_yaw);

    motor_pos_done = 0;
    motor_angle_done = 0;
}

void Set_Position_Angle_SP(float position_ref, float angle_ref)
{
    App_Update_Data();

    position_mid_ref = position_mid + position_ref;
    angle_yaw_ref = angle_ref;
    PID_ChangeSP(&pid_position_mid, position_mid_ref);
    PID_ChangeSP(&pid_angle_yaw, angle_yaw_ref);
    PID_Reset(&pid_position_mid);
    PID_Reset(&pid_angle_yaw);

    motor_pos_done = 0;
    motor_angle_done = 0;
}

void App_Angle_Pro(void)
{
    App_Update_Data();
    PERIODIC_START(ANGLE,30)

    if(App_Motor_Abs(App_Motor_AngleError(angle_yaw_ref, angle_yaw)) <= 0.7f){
        motor_angle_done = 1;
        motor_turn_speed = 0.0f;
        App_Speed_Set(0.0f, 0.0f);
        PID_Reset(&pid_angle_yaw);
    }else {
        motor_angle_done = 0;
        motor_turn_speed = App_Motor_PIDComputeAngle(&pid_angle_yaw, angle_yaw);

        if(motor_turn_speed > 0.0f && motor_turn_speed < 10.0f){
            motor_turn_speed = 5.0f;
        }else if(motor_turn_speed < 0.0f && motor_turn_speed > -10.0f){
            motor_turn_speed = -5.0f;
        }

        App_Speed_Set(motor_turn_speed, -motor_turn_speed);
    }
    PERIODIC_END

    App_Speed_Pro();
}

void App_Position_Pro(void)
{
    PERIODIC_START(POSITION,30)
    App_Update_Data();

    if(App_Motor_Abs(position_mid_ref - position_mid) <= 1.0f){
        motor_pos_done = 1;
        motor_base_speed = 0.0f;
        motor_left_speed = 0.0f;
        motor_right_speed = 0.0f;
        motor_turn_speed = 0.0f;
        App_Speed_Set(0.0f, 0.0f);
        PID_Reset(&pid_position_mid);
        PID_Reset(&pid_angle_yaw);
    }else {
        motor_pos_done = 0;
        motor_base_speed = App_Motor_PositionSpeed(position_mid_ref - position_mid);

        if(App_Motor_Abs(App_Motor_AngleError(angle_yaw_ref, angle_yaw)) <= 0.7f){
            motor_angle_done = 1;
            motor_turn_speed = 0.0f;
        }else {
            motor_angle_done = 0;
            motor_turn_speed = App_Motor_PIDComputeAngle(&pid_angle_yaw, angle_yaw);
        }

        motor_left_speed = motor_base_speed + motor_turn_speed;
        motor_right_speed = motor_base_speed - motor_turn_speed;
        App_Speed_Set(motor_left_speed, motor_right_speed);
    }
    PERIODIC_END

    App_Speed_Pro();
}
