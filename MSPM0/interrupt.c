#include "ti_msp_dl_config.h"
#include "interrupt.h"
#include "clock.h"
//#include "vl53l0x.h"
#include "imu.h"

uint8_t enable_group1_irq = 0;

void Interrupt_Init(void)
{
    if(enable_group1_irq)
    {
        NVIC_EnableIRQ(1);
    }
}

void SysTick_Handler(void)
{
    tick_ms++;
}

extern volatile unsigned char uart_data;
extern void Modbus_ParseFrame(uint8_t data);

// 电机控制串口接收中断服务函数
void MSPMotor_INST_IRQHandler(void){
    switch( DL_UART_getPendingInterrupt(MSPMotor_INST) )
    {
        case DL_UART_IIDX_RX:


                uart_data = DL_UART_Main_receiveData(MSPMotor_INST);

                Modbus_ParseFrame(uart_data);


            break;

        default:
            break;
    }
}
extern volatile unsigned char imu_data;
void IMU_INST_IRQHandler(void)
{
    switch( DL_UART_getPendingInterrupt(IMU_INST) )
    {
        case DL_UART_IIDX_RX: // 接收中断
            // 读取 IMU 串口数据并送入姿态解析器
            CopeSerial2Data(DL_UART_Main_receiveData(IMU_INST));
            break;

        default:
            break;
    }
}


// TIMER_0 1ms 定时器中断服务函数，当前预留
void TIMER_0_INST_IRQHandler(void)
{
    switch( DL_TimerG_getPendingInterrupt(TIMER_0_INST) )
    {
        case DL_TIMER_IIDX_ZERO:

            break;
        default:
            break;
    }
}


