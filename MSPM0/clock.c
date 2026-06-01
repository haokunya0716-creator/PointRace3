#include "ti_msp_dl_config.h"
#include "clock.h"

//volatile unsigned long tick_ms;
//volatile uint32_t start_time;

//int mspm0_delay_ms(unsigned long num_ms)
//{
//    start_time = tick_ms;
//    while (tick_ms - start_time < num_ms);
//    return 0;
//}

volatile unsigned long tick_ms;
// 删除：volatile uint32_t start_time; (把这行删掉)

int mspm0_delay_ms(unsigned long num_ms)
{
    // 改为局部变量，每次调用都是独立的，绝对安全
    unsigned long start_time_local = tick_ms; 
    while (tick_ms - start_time_local < num_ms);
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)
{
    if (!count)
        return 1;
    count[0] = tick_ms;
    return 0;
}

uint32_t HAL_GetTick(void)
{
    return (uint32_t)tick_ms;
}

void SysTick_Init(void)
{
    DL_SYSTICK_config(CPUCLK_FREQ/1000);
    // 【修改点1】将 SysTick 优先级从 0(最高) 降为 2，不要和串口抢CPU
    NVIC_SetPriority(SysTick_IRQn, 2);

    /* 硬件定时器TIMERA0中断配置 */
    // 【修改点2】也将 Timer0 的优先级设为 3，留出 0 和 1 给重要的通信串口
    NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 3); 
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerA_startCounter(TIMER_0_INST);
}