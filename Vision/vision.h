#pragma once

#include <stdint.h>
#include "ti_msp_dl_config.h"

// 创建结构体
struct Vision_Data{
    
  volatile uint8_t cmd;        // 命令位
  volatile uint8_t data;    // 数据位
    
};

extern struct Vision_Data vision;

// 函数声明
void Vision_Init(void);


void Vision_InvokeCallback(const uint8_t* rx_buf,uint8_t size);


void Vision_SendCommand(uint8_t cmd);


