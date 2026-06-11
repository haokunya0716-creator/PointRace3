#include "ti_msp_dl_config.h"
#include "interrupt.h"
#include "clock.h"
//#include "vl53l0x.h"
#include "imu.h"
#include "vision.h"

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

// 相机串口中断服务函数 
//void Vision_INST_IRQHandler(void)
//{
//    static uint8_t rx_buf[8];  
//    static uint8_t rx_idx = 0; 

//    uint32_t pending_irq = DL_UART_getPendingInterrupt(Vision_INST);

//    if (pending_irq == DL_UART_IIDX_RX) 
//    {
//        while (DL_UART_isRXFIFOEmpty(Vision_INST) == false) 
//        {
//            uint8_t temp_data = DL_UART_Main_receiveData(Vision_INST);

//            if (rx_idx == 0) {
//                if (temp_data == 0xAA) {
//                    rx_buf[rx_idx++] = temp_data;
//                }
//            } 
//            else if (rx_idx == 1) {
//                if (temp_data == 0x55) {
//                    rx_buf[rx_idx++] = temp_data;
//                } else {
//                    rx_idx = 0;
//                    if (temp_data == 0xAA) {
//                        rx_buf[rx_idx++] = temp_data;
//                    }
//                }
//            } 
//            else {
//                rx_buf[rx_idx++] = temp_data;
//                
//                if (rx_idx >= 5) {
//                    Vision_InvokeCallback(rx_buf, rx_idx);
//                    DL_GPIO_togglePins(LED_LED_2_PORT, LED_LED_2_PIN);
//                    rx_idx = 0; 
//                }
//            }
//        } 
//    }
//    else 
//    {
//        DL_UART_clearInterruptStatus(Vision_INST, 0xFFFFFFFF);
//    }
//}


 //相机串口中断服务函数（旧版本）
void Vision_INST_IRQHandler(void)
{

    static uint8_t rx_buf[8];  // 接收缓存数组
    static uint8_t rx_idx = 0;  // 接收索引

    switch (DL_UART_getPendingInterrupt(Vision_INST)) {
        case DL_UART_IIDX_RX:
          
            rx_buf[rx_idx] = DL_UART_Main_receiveData(Vision_INST);
            // 判断帧头
            if (rx_idx == 0 && rx_buf[0] != 0xAA) {
                rx_idx = 0; 
                break;
            }
            if (rx_idx == 1 && rx_buf[1] != 0x55) {
                rx_idx = 0; 
                break;
            }

         
            rx_idx++;
            // 当接受到5个字节（两个帧头，一个命令位，一个数据位，一个校验和帧尾）开始解析（回调函数里有计算校验和的逻辑）
            if (rx_idx >= 5) {
                
                Vision_InvokeCallback(rx_buf, rx_idx);
                DL_GPIO_togglePins(LED_LED_2_PORT, LED_LED_2_PIN);
                rx_idx = 0; 
            }
            break;
            
        default:
            
        // AI加的修复
             DL_UART_clearInterruptStatus(Vision_INST, 0xFFFFFFFF);
            
            // 顺便排空可能堆积的错误 FIFO 数据
            while (DL_UART_isRXFIFOEmpty(Vision_INST) == false) {
                DL_UART_Main_receiveData(Vision_INST); 
            }
            
            // 一旦有干扰，立刻复位接收状态机，防止后续数据全部错位
            rx_idx = 0; 
            break;
    }
}
// IMU陀螺仪 串口接收中断服务函数
void IMU_INST_IRQHandler(void)
{
    uint32_t pending_irq = DL_UART_getPendingInterrupt(IMU_INST);
    
    switch( pending_irq )
    {
        case DL_UART_IIDX_RX: // 正常接收中断
            // 读取 IMU 串口数据并送入姿态解析器
            CopeSerial2Data(DL_UART_Main_receiveData(IMU_INST));
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

// 当前电机驱动改为 AT8236，本串口不再解析旧电机 Modbus 数据。
void MSPMotor_INST_IRQHandler(void){
    uint32_t pending_irq = DL_UART_getPendingInterrupt(MSPMotor_INST);
    
    switch( pending_irq )
    {
        case DL_UART_IIDX_RX:
            DL_UART_Main_receiveData(MSPMotor_INST);
            break;

//        // 电机串口的免死金牌         
//        case DL_UART_IIDX_OVERRUN_ERROR:    
//        case DL_UART_IIDX_FRAMING_ERROR:    
//        case DL_UART_IIDX_PARITY_ERROR:     
//            DL_UART_clearInterruptStatus(MSPMotor_INST, 
//                DL_UART_INTERRUPT_OVERRUN_ERROR | 
//                DL_UART_INTERRUPT_FRAMING_ERROR | 
//                DL_UART_INTERRUPT_PARITY_ERROR);
//            while (DL_UART_isRXFIFOEmpty(MSPMotor_INST) == false) {
//                DL_UART_Main_receiveData(MSPMotor_INST); 
//            }
//            break;

        default:
            break;
    }
}

// TIMER_0 1ms 定时器中断服务函数，当前预留
void TIMER_0_INST_IRQHandler(void)
{
    switch( DL_Timer_getPendingInterrupt(TIMER_0_INST) )
    {
        case DL_TIMER_IIDX_ZERO:
            break;
        default:
            break;
    }
}