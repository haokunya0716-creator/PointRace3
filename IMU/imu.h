#ifndef __IMU_H__
#define __IMU_H__

#include "ti_msp_dl_config.h"
#include <stdio.h>

struct SAngle
{
    volatile float Yaw; // 使用 volatile 确保主函数每次都从内存中读取最新值
};
extern struct SAngle stcAngle;

struct SGyro
{
    short rawWz;
    volatile float wz;  // 使用 volatile
};
extern struct SGyro stcGyro;

void IMU_Init(void);
void IMU_send_char(char ch);
void IMU_send_string(char* str);
void IMU_send_bytes(uint8_t* data, uint32_t len);
void CopeSerial2Data(unsigned char ucData);

float getGyroZ(void);
float getYaw(void);
void sendCaliYawCommand(void);
void performCaliBias(void);

void delay_us(int __us);
void delay_ms(int __ms);

#endif
