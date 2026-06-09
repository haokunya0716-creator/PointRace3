//
// Created by gaoxu on 2026/2/7.
//

#include "app_encoder.h"
#include "app_motor.h"
#include "clock.h"
#include <stdint.h>

static volatile int64_t encoder_L = 0;
static volatile int64_t encoder_R = 0;

static volatile int8_t direction_l = 1;
static volatile int8_t direction_r = 1;

static volatile uint64_t t0_l = 0;
static volatile uint64_t t1_l = 0;

static volatile uint64_t t0_r = 0;
static volatile uint64_t t1_r = 0;

void App_Encoder_Init(void)
{
    DL_GPIO_clearInterruptStatus(GPIOA, ENCODER_E1A_PIN | ENCODER_E1B_PIN);
    DL_GPIO_clearInterruptStatus(GPIOB, ENCODER_E2A_PIN | ENCODER_E2B_PIN);

    NVIC_ClearPendingIRQ(ENCODER_GPIOA_INT_IRQN);
    NVIC_ClearPendingIRQ(ENCODER_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIOB_INT_IRQN);
}

static uint64_t App_Encoder_GetUs(void)
{
    unsigned long ms_1 = 0;
    unsigned long ms_2 = 0;
    uint32_t systick_value = 0;
    uint32_t systick_period = 0;
    uint32_t elapsed_ticks = 0;

    do {
        ms_1 = tick_ms;
        systick_value = DL_SYSTICK_getValue();
        ms_2 = tick_ms;
    } while (ms_1 != ms_2);

    systick_period = DL_SYSTICK_getPeriod();
    elapsed_ticks = systick_period - systick_value;

    return (uint64_t)ms_1 * 1000ULL +
           (uint64_t)elapsed_ticks * 1000000ULL / CPUCLK_FREQ;
}

float App_Encoder_GetPos_L(void)
{
    return encoder_L / MOTOR_AB_COUNT_PER_REV / MOTOR_REDUCTION * 360.0f;
}

float App_Encoder_GetPos_R(void)
{
    return encoder_R / MOTOR_AB_COUNT_PER_REV / MOTOR_REDUCTION * 360.0f;
}

float App_Encoder_GetSpeed_L(void)
{
    __disable_irq();
    int8_t direction_cpy = direction_l;
    uint64_t t0_r_cpy = t0_l;
    uint64_t t1_r_cpy = t1_l;
    __enable_irq();

    if (direction_cpy == 2 || direction_cpy == -2) {
        return 0.0f;
    } else {
        uint64_t now = App_Encoder_GetUs();
        float T;
        if (t0_r_cpy - t1_r_cpy > now - t0_r_cpy) {
            T = (t0_r_cpy - t1_r_cpy) * 1.0e-6f;
        } else {
            T = (now - t0_r_cpy) * 1.0e-6f;
        }
        return 1.0f * direction_cpy / T /
               MOTOR_AB_COUNT_PER_REV / MOTOR_REDUCTION *
               PI * MOTOR_WHEEL_DIA_CM;
    }
}

float App_Encoder_GetSpeed_R(void)
{
    __disable_irq();
    int8_t direction_cpy = direction_r;
    uint64_t t0_r_cpy = t0_r;
    uint64_t t1_r_cpy = t1_r;
    __enable_irq();

    if (direction_cpy == 2 || direction_cpy == -2) {
        return 0.0f;
    } else {
        uint64_t now = App_Encoder_GetUs();
        float T;
        if (t0_r_cpy - t1_r_cpy > now - t0_r_cpy) {
            T = (t0_r_cpy - t1_r_cpy) * 1.0e-6f;
        } else {
            T = (now - t0_r_cpy) * 1.0e-6f;
        }
        return 1.0f * direction_cpy / T /
               MOTOR_AB_COUNT_PER_REV / MOTOR_REDUCTION *
               PI * MOTOR_WHEEL_DIA_CM;
    }
}

void GROUP1_IRQHandler(void)
{
    uint32_t gpioa_status = DL_GPIO_getEnabledInterruptStatus(
        GPIOA, ENCODER_E1A_PIN | ENCODER_E1B_PIN);
    uint32_t gpiob_status = DL_GPIO_getEnabledInterruptStatus(
        GPIOB, ENCODER_E2A_PIN | ENCODER_E2B_PIN);

    if ((gpioa_status & ENCODER_E1A_PIN) != 0U) {
        t1_l = t0_l;
        t0_l = App_Encoder_GetUs();

        uint8_t a_L = (DL_GPIO_readPins(ENCODER_E1A_PORT, ENCODER_E1A_PIN) != 0U) ? 1U : 0U;
        uint8_t b_L = (DL_GPIO_readPins(ENCODER_E1B_PORT, ENCODER_E1B_PIN) != 0U) ? 1U : 0U;

        if ((a_L == 1U && b_L == 0U) || (a_L == 0U && b_L == 1U)) {
            encoder_L++;

            if (direction_l < 0) {
                direction_l = 2;
            } else {
                direction_l = 1;
            }
        } else {
            encoder_L--;

            if (direction_l > 0) {
                direction_l = -2;
            } else {
                direction_l = -1;
            }
        }
    }

    if ((gpiob_status & ENCODER_E2A_PIN) != 0U) {
        t1_r = t0_r;
        t0_r = App_Encoder_GetUs();

        uint8_t a_R = (DL_GPIO_readPins(ENCODER_E2A_PORT, ENCODER_E2A_PIN) != 0U) ? 1U : 0U;
        uint8_t b_R = (DL_GPIO_readPins(ENCODER_E2B_PORT, ENCODER_E2B_PIN) != 0U) ? 1U : 0U;

        if ((a_R == 1U && b_R == 0U) || (a_R == 0U && b_R == 1U)) {
            encoder_R--;

            if (direction_r > 0) {
                direction_r = -2;
            } else {
                direction_r = -1;
            }
        } else {
            encoder_R++;

            if (direction_r < 0) {
                direction_r = 2;
            } else {
                direction_r = 1;
            }
        }
    }

    DL_GPIO_clearInterruptStatus(GPIOA, gpioa_status);
    DL_GPIO_clearInterruptStatus(GPIOB, gpiob_status);
}

int App_Encoder_GetEncoder_L(void){

	return encoder_L;
}
int App_Encoder_GetEncoder_R(void){

	return encoder_R;
}
