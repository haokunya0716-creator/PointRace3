//
// Created by gaoxu on 2026/2/7.
//

#ifndef BALANCE_CAR_APP_ENCODER_H
#define BALANCE_CAR_APP_ENCODER_H

#include "ti_msp_dl_config.h"

void App_Encoder_Init(void);

float App_Encoder_GetPos_L(void);//读取电机转的角度值
float App_Encoder_GetPos_R(void);

float App_Encoder_GetSpeed_L(void);//T法测速，左电机
float App_Encoder_GetSpeed_R(void);//单位：cm/s

int App_Encoder_GetEncoder_L(void);
int App_Encoder_GetEncoder_R(void);


#endif //BALANCE_CAR_APP_ENCODER_H
