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

// 电机控制串口中断服务函数
void MSPMotor_INST_IRQHandler(void)
{
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

void IMU_INST_IRQHandler(void)
{
    switch( DL_UART_getPendingInterrupt(IMU_INST) )
    {
        case DL_UART_IIDX_RX: // 接收中断
            // 直接读取数据并送入 board.c 的解析器
            CopeSerial2Data(DL_UART_Main_receiveData(IMU_INST));
            break;

        default:
            break;
    }
}


// Timer_0 1ms定时器中断服务函数 (若需保留)
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

/*
void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1)) {
        #if defined GPIO_MULTIPLE_GPIOA_INT_IIDX
        case GPIO_MULTIPLE_GPIOA_INT_IIDX:
            switch (DL_GPIO_getPendingInterrupt(GPIOA))
            {
                #if (defined GPIO_MPU6050_PORT) && (GPIO_MPU6050_PORT == GPIOA)
                case GPIO_MPU6050_PIN_MPU6050_INT_IIDX:
                    Read_Quad();
                    break;
                #endif

                #if (defined GPIO_LSM6DSV16X_PORT) && (GPIO_LSM6DSV16X_PORT == GPIOA)
                case GPIO_LSM6DSV16X_PIN_LSM6DSV16X_INT_IIDX:
                    Read_LSM6DSV16X();
                    break;
                #endif

                #if (defined GPIO_VL53L0X_PIN_VL53L0X_GPIO1_PORT) && (GPIO_VL53L0X_PIN_VL53L0X_GPIO1_PORT == GPIOA)
                case GPIO_VL53L0X_PIN_VL53L0X_GPIO1_IIDX:
                    Read_VL53L0X();
                    break;
                #endif

                default:
                    break;
            }
        #endif

        #if defined GPIO_VL53L0X_INT_IIDX
            case GPIO_VL53L0X_INT_IIDX:
                Read_VL53L0X();
                break;
        #endif
    }
}
*/