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

// IMU 串口接收中断服务函数
void IMU_INST_IRQHandler(void)
{
    uint32_t pending_irq = DL_UART_getPendingInterrupt(IMU_INST);
    
    switch( pending_irq )
    {
        case DL_UART_IIDX_RX: // 正常接收中断
            // 读取 IMU 串口数据并送入姿态解析器
            CopeSerial2Data(DL_UART_Main_receiveData(IMU_INST));
             DL_GPIO_togglePins(LED_LED_0_PORT, LED_LED_0_PIN);
            break;

        // 【修改点5：核心救命代码】加入硬件错误处理，防止死锁
        case DL_UART_IIDX_OVERRUN_ERROR:    // 溢出错误 (delay_ms导致的真凶)
        case DL_UART_IIDX_FRAMING_ERROR:    // 帧错误
        case DL_UART_IIDX_PARITY_ERROR:     // 校验错误
            // 1. 清除所有的错误中断标志位，这步不写，串口永久假死
            DL_UART_clearInterruptStatus(IMU_INST, 
                DL_UART_INTERRUPT_OVERRUN_ERROR | 
                DL_UART_INTERRUPT_FRAMING_ERROR | 
                DL_UART_INTERRUPT_PARITY_ERROR);
            
            // 2. 把坏掉的数据强行读出来扔掉，疏通 FIFO
            while (DL_UART_isRXFIFOEmpty(IMU_INST) == false) {
                DL_UART_Main_receiveData(IMU_INST); 
            }
            break;

        default:
            break;
    }
}

// 电机控制串口接收中断服务函数 (为了防止电机串口以后也死机，一起加上保护)
void MSPMotor_INST_IRQHandler(void){
    uint32_t pending_irq = DL_UART_getPendingInterrupt(MSPMotor_INST);
    
    switch( pending_irq )
    {
        case DL_UART_IIDX_RX:
            uart_data = DL_UART_Main_receiveData(MSPMotor_INST);
            Modbus_ParseFrame(uart_data);
            break;

        // 电机串口的免死金牌         
        case DL_UART_IIDX_OVERRUN_ERROR:    
        case DL_UART_IIDX_FRAMING_ERROR:    
        case DL_UART_IIDX_PARITY_ERROR:     
            DL_UART_clearInterruptStatus(MSPMotor_INST, 
                DL_UART_INTERRUPT_OVERRUN_ERROR | 
                DL_UART_INTERRUPT_FRAMING_ERROR | 
                DL_UART_INTERRUPT_PARITY_ERROR);
            while (DL_UART_isRXFIFOEmpty(MSPMotor_INST) == false) {
                DL_UART_Main_receiveData(MSPMotor_INST); 
            }
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
//DL_GPIO_togglePins(LED_LED_0_PORT, LED_LED_0_PIN);
            break;
        default:
            break;
    }
}


