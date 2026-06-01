#include "app_motor.h"
#include "clock.h"
#include "imu.h"
#include "task.h"
#include "motor_read_enc.h"
#include "motor_set_speed.h"
#include "imu.h"
static PID_TypeDef pid_position_l; // 电机位置闭环的PID控制器
static PID_TypeDef pid_position_r; 

extern volatile int16_t	modbus_date[8];//电机反馈		

float motor_right_speed,motor_left_speed = 0;//电机速度

int16_t motor_encoder1,motor_encoder2 = 0;//编码器值
float position_l,position_r = 0;//累计走过的c

float position_l_ref,position_r_ref = 0;//位置期望值


static PID_TypeDef pid_angle_yaw;
volatile float angle_yaw = 0;//当前的角度值
//
// @简介：速度转换，设置速度的单位为r/s，反馈值为编码器每30ms
//
static float compute_speed_input(float speed_ref){
	
	return speed_ref * 0.03f * 30.0f * 13.0f;
		
}

//
// @简介：速度转换，经过转换后反馈速度的单位为r/s
//
static float compute_speed_output(float speed_measure_encoder){
	
	return speed_measure_encoder / 0.03f / 30.0f / 13.0f;
	
}

//
// @简介：小车走的距离与编码器转换，输入的单位是cm，反馈值为距离对应的编码器变化值
//
static float compute_position_input(float position_ref){
	
	return position_ref / PI / 6.5f * 30.0f * 13.0f;

}

//
// @简介：小车走的距离与编码器转换，输入的是编码器，反馈值为距离，单位cm
//
static float compute_position_output(float position_measure){
	
	return position_measure * PI * 6.5f / 30.0f / 13.0f;

}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//
// @简介：初始化左右电机pid系统
//
void App_Motor_Init(void)
{

    PID_Init(&pid_position_l,0.0900, 0.0, 0);
    PID_LimitConfig(&pid_position_l, +500.0f, -500.0f);
	  PID_Init(&pid_position_r, 0.09, 0.0, 0);
    PID_LimitConfig(&pid_position_r, +500.0f, -500.0f);
	
		PID_Init(&pid_angle_yaw, 0.12, 0.04, 0);
    PID_LimitConfig(&pid_angle_yaw, +500.0f, -500.0f);


}

//
// @简介：更新陀螺仪角度和实际走过的距离
//
void App_Update_Data(void){

	angle_yaw = getYaw();

	motor_encoder1 = modbus_date[0];//左右电机的编码器值
	motor_encoder2 = modbus_date[1];
	
	position_l = compute_position_output(motor_encoder1);//左右电机累积走过的距离（cm）
	position_r = compute_position_output(motor_encoder2);
	
}

void App_PID_Reset(void){

	PID_Reset(&pid_position_l);
	PID_Reset(&pid_position_r);
}
//
//@brief:设置左电机目标值(相对)，（设置单位：cm）
//
void Set_Position_SP_L(float position_ref){

	pid_position_l.SP = position_l - position_ref;
	
}

//
//@brief:设置右电机目标值（相对），（设置单位：cm）
//
void Set_Position_SP_R(float position_ref){

	pid_position_r.SP = position_r + position_ref;
	
}
void App_Position_Pro(void){
	
	
	PERIODIC_START(POSITION,30)
	App_Update_Data();
	
	
	motor_left_speed =  PID_Compute(&pid_position_l,position_l);
	motor_right_speed = PID_Compute(&pid_position_r,position_r);
	
	
	float speedl_raw = compute_speed_input(motor_left_speed);
	float speedr_raw = compute_speed_input(motor_right_speed);
	
	Motor_Set_Speeds(speedl_raw,speedr_raw,0,0);
	
	PERIODIC_END
	
	PERIODIC_START(PRINT_POSITION,200)
	printf("%f,%f,%.2f\n",pid_position_r.SP,position_r, stcAngle.Yaw);

	PERIODIC_END
}
//两个轮子中间的直线距离为20.5cm
//pi/2*20.5
void Turn_Right_90(void){//向右转90度
	
	uint32_t time = HAL_GetTick();
	while(HAL_GetTick() - time < 880){
		PERIODIC_START(TURN_RIGHT,30)
			App_Set_Speed(1.0f,-1.0f);
		PERIODIC_END
	}
}
void Turn_Left_90(void){//向左转90°

	float encoder_r_change = PI * 9.0f;
	Set_Position_SP_R(encoder_r_change);
	
}
//
//@brief：单位r/s
//
void App_Set_Speed(float speed_l,float speed_r){

	float speed_l_raw = - speed_l * 0.03f * 30.0f * 13.0f;
	float speed_r_raw = speed_r * 0.03f * 30.0f * 13.0f;
	
		Motor_Set_Speeds(speed_l_raw,speed_r_raw,0,0);
			
}
/**
 * @brief 把角度误差限制到 [-180, 180]，避免跨越 ±180° 时误差突变
 * @param target 目标角度
 * @param current 当前角度
 * @return 处理后的误差值。
 */
float Angle_Error(float target, float current)
{
    float err = target - current;

    while(err > 180)  err -= 360;
    while(err < -180) err += 360;

    return err;
}
/**
 * @brief 执行一次角度 PID 计算，返回控制量（电压差）。

 */
float PID_Compute_angle(PID_TypeDef *PID, float FB)
{
    float err = Angle_Error(PID->SP, FB);

		unsigned long now_ms = 0;
		mspm0_get_clock_ms(&now_ms);
		uint64_t t_k = now_ms;

		unsigned long delta_ms = now_ms - (unsigned long)PID->t_k_1;
		float deltaT = delta_ms * 1.0e-3f;
		if(deltaT <= 0.0f) deltaT = 1.0e-3f;

    //首次运行时忽略积分项和微分项
    float err_dev = 0.0f;
    float err_int = 0.0f;

    if(PID->t_k_1 != 0)
    {
        err_dev = (err - PID->err_k_1) / deltaT;
        //这是定速控制的，不需要积分分离
        err_int = PID->err_int_k_1 + (err + PID->err_k_1) * deltaT * 0.5f;

        // //定位置控制时，需要加入积分分离
        //阈值的大小，先用pd控制器测几次稳态，误差最大值设为阈值
        // if ( fabsf(err) < 积分分离阈值) {
        //  err_int = PID->err_int_k_1 + (err + PID->err_k_1) * deltaT * 0.5f;
        // }else {
        //  err_int = 0;
        // }
        if(fabsf(err) < 5.0f)
        {
            err_int = PID->err_int_k_1 + (err + PID->err_k_1) * deltaT * 0.5f;
        }
        else
        {
            err_int = 0;
        }
    }

    float COp = PID->Kp * err;
    float COi = PID->Ki * err_int;
    float COd = PID->Kd * err_dev;
    float CO = COp + COi + COd;

    // 更新
    PID->t_k_1 = t_k;
    PID->err_k_1 = err;
    PID->err_int_k_1 = err_int;

    // 输出限幅
    if(CO > PID->UpperLimit) CO = PID->UpperLimit;
    if(CO < PID->LowerLimit) CO = PID->LowerLimit;


    if(PID->err_int_k_1 > PID->UpperLimit)
        PID->err_int_k_1 = PID->UpperLimit;
    if(PID->err_int_k_1 < PID->LowerLimit)
        PID->err_int_k_1 = PID->LowerLimit;

    return CO;
}
//
//@brief:设置角度闭环的目标值（相对值），单位：°
//
void Set_Angle_SP(float angle_ref){

	pid_angle_yaw.SP = angle_yaw + angle_ref;
	
}
void App_Angle_Pro(void){
	PERIODIC_START(PRINT,100)
	printf("%f,%f\n",pid_angle_yaw.SP,angle_yaw);
	PERIODIC_END
	PERIODIC_START(ANGLE,25)
	
	float change_speed = PID_Compute_angle(&pid_angle_yaw,angle_yaw);
	
	App_Set_Speed(change_speed,-change_speed);
	PERIODIC_END
}


