#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "ti_msp_dl_config.h"

void Motor_Init(void);
void Motor_Cmd(uint8_t on);
void Motor_Set_L(float duty);
void Motor_Set_R(float duty);
void Motor_Stop_L(void);
void Motor_Stop_R(void);
void Motor_Stop(void);

#endif
