//
// Created by gaoxu on 2026/2/4.
//

#include "app_bat.h"
#include "task.h"
#include "hardware/sast_lib_st7735s.h"

static volatile float volt = 0; // 电池电压，单位V
static volatile uint16_t volt100 = 0; // 电池电压放大100倍，用于LCD显示

//
// @简介：初始化电池电压检测模块
//
void App_Bat_Init(void)
{
   NVIC_EnableIRQ(ADC_BAT_INST_INT_IRQN);
}

//
// @简介：获取电池电压结果
//
float App_Bat_Get(void)
{
    return volt;
}

//
// @简介：ADC转换完成中断函数，在中断里计算电池电压
//
void ADC_BAT_INST_IRQHandler(void)
{
    if (DL_ADC12_getPendingInterrupt(ADC_BAT_INST) == DL_ADC12_IIDX_MEM0_RESULT_LOADED)
    {
        uint16_t dr = DL_ADC12_getMemResult(ADC_BAT_INST, ADC_BAT_ADCMEM_0);
        float now = dr / 4095.0f * 36.3f;

        if (volt == 0)
            volt = now;
        else
            volt = volt * 0.9f + now * 0.1f;

        volt100 = (uint16_t)(volt * 100.0f + 0.5f);
			DL_GPIO_togglePins(LED_PORT, LED_LED_0_PIN);
			
			DL_ADC12_enableConversions(ADC_BAT_INST);
    }
}

//
// @简介：电池电压显示进程函数
//
void App_Bat_Proc(void)
{
    PERIODIC_START(task_bat, 1000)
        DL_ADC12_startConversion(ADC_BAT_INST);//开启ADC测量
	      //刷新屏幕显示
        st7735s_fill_rect(80, 40, ST7735S_W - 1, 55, ST7735S_BLACK);
        st7735s_printf(80, 40, ST7735S_WHITE, ST7735S_BLACK,
                       ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE,
                       "%d.%02dV", volt100 / 100, volt100 % 100);
    PERIODIC_END
}
