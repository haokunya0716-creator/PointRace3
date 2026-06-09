//
// Created by gaoxu on 2026/2/4.
//

#include "app_bat.h"
#include "task.h"

static volatile float volt = 0.0f;

void App_Bat_Init(void)
{
    NVIC_EnableIRQ(ADC_BAT_INST_INT_IRQN);
}

float App_Bat_Get(void)
{
    return volt;
}

void ADC_BAT_INST_IRQHandler(void)
{
    if (DL_ADC12_getPendingInterrupt(ADC_BAT_INST) == DL_ADC12_IIDX_MEM0_RESULT_LOADED)
    {
        uint16_t dr = DL_ADC12_getMemResult(ADC_BAT_INST, ADC_BAT_ADCMEM_0);
        float now = dr / 4095.0f * 36.3f;

        if (volt == 0.0f)
            volt = now;
        else
            volt = volt * 0.9f + now * 0.1f;

        DL_ADC12_enableConversions(ADC_BAT_INST);
    }
}

void App_Bat_Proc(void)
{
    PERIODIC_START(task_bat, 1000)
        DL_ADC12_startConversion(ADC_BAT_INST);
    PERIODIC_END
}
