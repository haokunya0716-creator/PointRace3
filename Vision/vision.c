#include "vision.h"

// 实例化接收数据结构体，调用示例vision.cmd/vision.date
struct Vision_Data vision;

// 打开串口2的接收中断，使能中断
void Vision_Init(void){
    // 把优先级调到最高
    NVIC_SetPriority(Vision_INST_INT_IRQN, 0);

	// 清除错误标志和上次残留数据
    DL_UART_clearInterruptStatus(Vision_INST, 0xFFFFFFFF); 
    while (DL_UART_isRXFIFOEmpty(Vision_INST) == false) {
        DL_UART_Main_receiveData(Vision_INST); 
    }
    
    DL_UART_enableInterrupt(Vision_INST, DL_UART_INTERRUPT_RX);

    NVIC_ClearPendingIRQ(Vision_INST_INT_IRQN);
    NVIC_EnableIRQ(Vision_INST_INT_IRQN);

}
// 辅助函数，计算校验和，返回校验值
static uint8_t Vision_CalcChecksum(const uint8_t* data, uint8_t len){

    uint16_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

// 回调函数
void Vision_InvokeCallback(const uint8_t* rx_data, uint8_t size){
    
    // 判断校验和
    if(Vision_CalcChecksum(rx_data, 4) != rx_data[4]) {
        return; 
    }
       
    // 获取数据
    vision.cmd     = rx_data[2];
    vision.data = rx_data[3];
    
}
// 单片机向相机发送指令的命令
void Vision_SendCommand(uint8_t cmd)
{
  static uint8_t tx_buf[8];
    
  tx_buf[0] = 0xAA;
  tx_buf[1] = 0x55;  
  tx_buf[2] = cmd;
  tx_buf[3] = 0;  
  tx_buf[4] = Vision_CalcChecksum(tx_buf,4);
    
    // 逐字节循环阻塞发送
    for(uint8_t i = 0; i < 5; i++) {
        DL_UART_Main_transmitData(Vision_INST, tx_buf[i]); 
        while(DL_UART_isBusy(Vision_INST) == true); 
    }
}









