#ifndef __IMU_H__
#define __IMU_H__

#include "ti_msp_dl_config.h"
#include <stdio.h>

//陀螺仪角度,使用 volatile 确保主函数每次都从内存中读取最新值
struct SAngle
{
    
    volatile float Yaw; 
    volatile float Pitch;
    volatile float Roll;
};
extern struct SAngle Angle;

//陀螺仪角速度
struct SGyro
{
    short rawWx;
    volatile float wx;  // X轴角速度  (±2000°/s)
    
    short rawWy;
    volatile float wy;  // Y轴角速度  (±2000°/s)
    
    short rawWz;
    volatile float wz;  // Z轴角速度  (±2000°/s)
    
};
extern struct SGyro Gyro;

//陀螺仪加速度
struct SAccel
{
   
    volatile float ax;  // X轴加速度  (±16g)
    volatile float ay;  // Y轴加速度  (±16g)
    volatile float az;  // Z轴加速度  (±16g)
    
    volatile short rawAx;
    volatile short rawAy;
    volatile short rawAz;
};
extern struct SAccel Accel;
/**
 * @brief 四元数结构体 (归一化单位)
 */
struct SQuat
{
    float q0;     // 四元数 q0
    float q1;     // 四元数 q1
    float q2;     // 四元数 q2
    float q3;     // 四元数 q3
};
extern struct SQuat   Quat;    // 四元数数据

void IMU_Init(void);                          //初始化
void CopeSeriaIMU1Data(unsigned char ucData);   //单轴陀螺仪的解析函数
void CopeSeriaIMU6Data(unsigned char ucData);   //六轴陀螺仪的解析函数

//获取角速度
float getGyroX(void);
float getGyroY(void);
float getGyroZ(void);

//获取角度
float getYaw(void);
float getPitch(void);
float getRoll(void);
//校准函数
void sendCaliYawCommand1(void);
void sendCaliYawCommand6(void);
void performCaliBias(void);

void delay_us(int __us);
void delay_ms(int __ms);

#endif