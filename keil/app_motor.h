#ifndef APP_MOTOR_H_
#define APP_MOTOR_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "pid.h"

#define WHEEL_DIA 65.0f //轮子直径单位mm
#define MOTOR_REDUCTION 30 //减速比
#define PI 3.1415926
//左电机向前负，右电机向前正
//陀螺仪顺时针转为正

void App_Motor_Init(void);
void App_Update_Data(void);

void Set_Position_SP_L(float position_ref);
void Set_Position_SP_R(float position_ref);
void Set_Angle_SP(float angle_ref);//顺时针为正方向
void App_Position_Pro(void); 
void App_Angle_Pro(void); 
void App_Set_Speed(float speed_l,float speed_r); //单位：r/s
void Turn_Right_90(void);//向右转90度
void Turn_Left_90(void);//向左转90°
#endif
