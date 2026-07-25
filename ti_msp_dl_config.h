/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     80000000



/* Defines for MOTOR */
#define MOTOR_INST                                                         TIMA0
#define MOTOR_INST_IRQHandler                                   TIMA0_IRQHandler
#define MOTOR_INST_INT_IRQN                                     (TIMA0_INT_IRQn)
#define MOTOR_INST_CLK_FREQ                                             80000000
/* GPIO defines for channel 0 */
#define GPIO_MOTOR_C0_PORT                                                 GPIOA
#define GPIO_MOTOR_C0_PIN                                          DL_GPIO_PIN_8
#define GPIO_MOTOR_C0_IOMUX                                      (IOMUX_PINCM19)
#define GPIO_MOTOR_C0_IOMUX_FUNC                     IOMUX_PINCM19_PF_TIMA0_CCP0
#define GPIO_MOTOR_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_MOTOR_C1_PORT                                                 GPIOA
#define GPIO_MOTOR_C1_PIN                                          DL_GPIO_PIN_9
#define GPIO_MOTOR_C1_IOMUX                                      (IOMUX_PINCM20)
#define GPIO_MOTOR_C1_IOMUX_FUNC                     IOMUX_PINCM20_PF_TIMA0_CCP1
#define GPIO_MOTOR_C1_IDX                                    DL_TIMER_CC_1_INDEX
/* GPIO defines for channel 2 */
#define GPIO_MOTOR_C2_PORT                                                 GPIOB
#define GPIO_MOTOR_C2_PIN                                         DL_GPIO_PIN_12
#define GPIO_MOTOR_C2_IOMUX                                      (IOMUX_PINCM29)
#define GPIO_MOTOR_C2_IOMUX_FUNC                     IOMUX_PINCM29_PF_TIMA0_CCP2
#define GPIO_MOTOR_C2_IDX                                    DL_TIMER_CC_2_INDEX
/* GPIO defines for channel 3 */
#define GPIO_MOTOR_C3_PORT                                                 GPIOB
#define GPIO_MOTOR_C3_PIN                                         DL_GPIO_PIN_13
#define GPIO_MOTOR_C3_IOMUX                                      (IOMUX_PINCM30)
#define GPIO_MOTOR_C3_IOMUX_FUNC                     IOMUX_PINCM30_PF_TIMA0_CCP3
#define GPIO_MOTOR_C3_IDX                                    DL_TIMER_CC_3_INDEX

/* Defines for SERVO3 */
#define SERVO3_INST                                                        TIMG0
#define SERVO3_INST_IRQHandler                                  TIMG0_IRQHandler
#define SERVO3_INST_INT_IRQN                                    (TIMG0_INT_IRQn)
#define SERVO3_INST_CLK_FREQ                                              500000
/* GPIO defines for channel 1 */
#define GPIO_SERVO3_C1_PORT                                                GPIOB
#define GPIO_SERVO3_C1_PIN                                        DL_GPIO_PIN_11
#define GPIO_SERVO3_C1_IOMUX                                     (IOMUX_PINCM28)
#define GPIO_SERVO3_C1_IOMUX_FUNC                    IOMUX_PINCM28_PF_TIMG0_CCP1
#define GPIO_SERVO3_C1_IDX                                   DL_TIMER_CC_1_INDEX



/* Defines for TIMER_0 */
#define TIMER_0_INST                                                     (TIMA1)
#define TIMER_0_INST_IRQHandler                                 TIMA1_IRQHandler
#define TIMER_0_INST_INT_IRQN                                   (TIMA1_INT_IRQn)
#define TIMER_0_INST_LOAD_VALUE                                          (3999U)




/* Defines for I2C_VL53L0X */
#define I2C_VL53L0X_INST                                                    I2C0
#define I2C_VL53L0X_INST_IRQHandler                              I2C0_IRQHandler
#define I2C_VL53L0X_INST_INT_IRQN                                  I2C0_INT_IRQn
#define I2C_VL53L0X_BUS_SPEED_HZ                                          100000
#define GPIO_I2C_VL53L0X_SDA_PORT                                          GPIOA
#define GPIO_I2C_VL53L0X_SDA_PIN                                   DL_GPIO_PIN_0
#define GPIO_I2C_VL53L0X_IOMUX_SDA                                (IOMUX_PINCM1)
#define GPIO_I2C_VL53L0X_IOMUX_SDA_FUNC                 IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_I2C_VL53L0X_SCL_PORT                                          GPIOA
#define GPIO_I2C_VL53L0X_SCL_PIN                                   DL_GPIO_PIN_1
#define GPIO_I2C_VL53L0X_IOMUX_SCL                                (IOMUX_PINCM2)
#define GPIO_I2C_VL53L0X_IOMUX_SCL_FUNC                 IOMUX_PINCM2_PF_I2C0_SCL


/* Defines for MSPMotor */
#define MSPMotor_INST                                                      UART3
#define MSPMotor_INST_FREQUENCY                                          4000000
#define MSPMotor_INST_IRQHandler                                UART3_IRQHandler
#define MSPMotor_INST_INT_IRQN                                    UART3_INT_IRQn
#define GPIO_MSPMotor_RX_PORT                                              GPIOB
#define GPIO_MSPMotor_TX_PORT                                              GPIOB
#define GPIO_MSPMotor_RX_PIN                                       DL_GPIO_PIN_3
#define GPIO_MSPMotor_TX_PIN                                       DL_GPIO_PIN_2
#define GPIO_MSPMotor_IOMUX_RX                                   (IOMUX_PINCM16)
#define GPIO_MSPMotor_IOMUX_TX                                   (IOMUX_PINCM15)
#define GPIO_MSPMotor_IOMUX_RX_FUNC                    IOMUX_PINCM16_PF_UART3_RX
#define GPIO_MSPMotor_IOMUX_TX_FUNC                    IOMUX_PINCM15_PF_UART3_TX
#define MSPMotor_BAUD_RATE                                              (115200)
#define MSPMotor_IBRD_4_MHZ_115200_BAUD                                      (2)
#define MSPMotor_FBRD_4_MHZ_115200_BAUD                                     (11)
/* Defines for IMU */
#define IMU_INST                                                           UART1
#define IMU_INST_FREQUENCY                                              40000000
#define IMU_INST_IRQHandler                                     UART1_IRQHandler
#define IMU_INST_INT_IRQN                                         UART1_INT_IRQn
#define GPIO_IMU_RX_PORT                                                   GPIOB
#define GPIO_IMU_TX_PORT                                                   GPIOB
#define GPIO_IMU_RX_PIN                                            DL_GPIO_PIN_5
#define GPIO_IMU_TX_PIN                                            DL_GPIO_PIN_4
#define GPIO_IMU_IOMUX_RX                                        (IOMUX_PINCM18)
#define GPIO_IMU_IOMUX_TX                                        (IOMUX_PINCM17)
#define GPIO_IMU_IOMUX_RX_FUNC                         IOMUX_PINCM18_PF_UART1_RX
#define GPIO_IMU_IOMUX_TX_FUNC                         IOMUX_PINCM17_PF_UART1_TX
#define IMU_BAUD_RATE                                                   (115200)
#define IMU_IBRD_40_MHZ_115200_BAUD                                         (21)
#define IMU_FBRD_40_MHZ_115200_BAUD                                         (45)
/* Defines for Vision */
#define Vision_INST                                                        UART2
#define Vision_INST_FREQUENCY                                           40000000
#define Vision_INST_IRQHandler                                  UART2_IRQHandler
#define Vision_INST_INT_IRQN                                      UART2_INT_IRQn
#define GPIO_Vision_RX_PORT                                                GPIOB
#define GPIO_Vision_TX_PORT                                                GPIOB
#define GPIO_Vision_RX_PIN                                        DL_GPIO_PIN_16
#define GPIO_Vision_TX_PIN                                        DL_GPIO_PIN_15
#define GPIO_Vision_IOMUX_RX                                     (IOMUX_PINCM33)
#define GPIO_Vision_IOMUX_TX                                     (IOMUX_PINCM32)
#define GPIO_Vision_IOMUX_RX_FUNC                      IOMUX_PINCM33_PF_UART2_RX
#define GPIO_Vision_IOMUX_TX_FUNC                      IOMUX_PINCM32_PF_UART2_TX
#define Vision_BAUD_RATE                                                (115200)
#define Vision_IBRD_40_MHZ_115200_BAUD                                      (21)
#define Vision_FBRD_40_MHZ_115200_BAUD                                      (45)
/* Defines for user */
#define user_INST                                                          UART0
#define user_INST_FREQUENCY                                              4000000
#define user_INST_IRQHandler                                    UART0_IRQHandler
#define user_INST_INT_IRQN                                        UART0_INT_IRQn
#define GPIO_user_RX_PORT                                                  GPIOA
#define GPIO_user_TX_PORT                                                  GPIOA
#define GPIO_user_RX_PIN                                          DL_GPIO_PIN_11
#define GPIO_user_TX_PIN                                          DL_GPIO_PIN_10
#define GPIO_user_IOMUX_RX                                       (IOMUX_PINCM22)
#define GPIO_user_IOMUX_TX                                       (IOMUX_PINCM21)
#define GPIO_user_IOMUX_RX_FUNC                        IOMUX_PINCM22_PF_UART0_RX
#define GPIO_user_IOMUX_TX_FUNC                        IOMUX_PINCM21_PF_UART0_TX
#define user_BAUD_RATE                                                  (115200)
#define user_IBRD_4_MHZ_115200_BAUD                                          (2)
#define user_FBRD_4_MHZ_115200_BAUD                                         (11)




/* Defines for NRF */
#define NRF_INST                                                           SPI1
#define NRF_INST_IRQHandler                                     SPI1_IRQHandler
#define NRF_INST_INT_IRQN                                         SPI1_INT_IRQn
#define GPIO_NRF_PICO_PORT                                                GPIOB
#define GPIO_NRF_PICO_PIN                                         DL_GPIO_PIN_8
#define GPIO_NRF_IOMUX_PICO                                     (IOMUX_PINCM25)
#define GPIO_NRF_IOMUX_PICO_FUNC                     IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_NRF_POCI_PORT                                                GPIOB
#define GPIO_NRF_POCI_PIN                                        DL_GPIO_PIN_14
#define GPIO_NRF_IOMUX_POCI                                     (IOMUX_PINCM31)
#define GPIO_NRF_IOMUX_POCI_FUNC                     IOMUX_PINCM31_PF_SPI1_POCI
/* GPIO configuration for NRF */
#define GPIO_NRF_SCLK_PORT                                                GPIOB
#define GPIO_NRF_SCLK_PIN                                         DL_GPIO_PIN_9
#define GPIO_NRF_IOMUX_SCLK                                     (IOMUX_PINCM26)
#define GPIO_NRF_IOMUX_SCLK_FUNC                     IOMUX_PINCM26_PF_SPI1_SCLK
#define GPIO_NRF_CS0_PORT                                                 GPIOA
#define GPIO_NRF_CS0_PIN                                          DL_GPIO_PIN_2
#define GPIO_NRF_IOMUX_CS0                                       (IOMUX_PINCM7)
#define GPIO_NRF_IOMUX_CS0_FUNC                        IOMUX_PINCM7_PF_SPI1_CS0



/* Defines for ADC_BAT */
#define ADC_BAT_INST                                                        ADC1
#define ADC_BAT_INST_IRQHandler                                  ADC1_IRQHandler
#define ADC_BAT_INST_INT_IRQN                                    (ADC1_INT_IRQn)
#define ADC_BAT_ADCMEM_0                                      DL_ADC12_MEM_IDX_0
#define ADC_BAT_ADCMEM_0_REF                     DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC_BAT_ADCMEM_0_REF_VOLTAGE_V                                       3.3
#define GPIO_ADC_BAT_C0_PORT                                               GPIOA
#define GPIO_ADC_BAT_C0_PIN                                       DL_GPIO_PIN_15



/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOB)

/* Defines for PIN_0: GPIOB.22 with pinCMx 50 on package pin 21 */
#define LED_PIN_0_PIN                                           (DL_GPIO_PIN_22)
#define LED_PIN_0_IOMUX                                          (IOMUX_PINCM50)
/* Defines for XSHUT1: GPIOA.17 with pinCMx 39 on package pin 10 */
#define XSHUT_XSHUT1_PORT                                                (GPIOA)
#define XSHUT_XSHUT1_PIN                                        (DL_GPIO_PIN_17)
#define XSHUT_XSHUT1_IOMUX                                       (IOMUX_PINCM39)
/* Defines for XSHUT2: GPIOB.0 with pinCMx 12 on package pin 47 */
#define XSHUT_XSHUT2_PORT                                                (GPIOB)
#define XSHUT_XSHUT2_PIN                                         (DL_GPIO_PIN_0)
#define XSHUT_XSHUT2_IOMUX                                       (IOMUX_PINCM12)
/* Defines for XSHUT3: GPIOA.30 with pinCMx 5 on package pin 37 */
#define XSHUT_XSHUT3_PORT                                                (GPIOA)
#define XSHUT_XSHUT3_PIN                                        (DL_GPIO_PIN_30)
#define XSHUT_XSHUT3_IOMUX                                        (IOMUX_PINCM5)
/* Defines for XSHUT0: GPIOB.19 with pinCMx 45 on package pin 16 */
#define XSHUT_XSHUT0_PORT                                                (GPIOB)
#define XSHUT_XSHUT0_PIN                                        (DL_GPIO_PIN_19)
#define XSHUT_XSHUT0_IOMUX                                       (IOMUX_PINCM45)
/* Defines for PIN1: GPIOA.26 with pinCMx 59 on package pin 30 */
#define LINE_PIN1_PORT                                                   (GPIOA)
#define LINE_PIN1_PIN                                           (DL_GPIO_PIN_26)
#define LINE_PIN1_IOMUX                                          (IOMUX_PINCM59)
/* Defines for PIN2: GPIOA.24 with pinCMx 54 on package pin 25 */
#define LINE_PIN2_PORT                                                   (GPIOA)
#define LINE_PIN2_PIN                                           (DL_GPIO_PIN_24)
#define LINE_PIN2_IOMUX                                          (IOMUX_PINCM54)
/* Defines for PIN3: GPIOB.25 with pinCMx 56 on package pin 27 */
#define LINE_PIN3_PORT                                                   (GPIOB)
#define LINE_PIN3_PIN                                           (DL_GPIO_PIN_25)
#define LINE_PIN3_IOMUX                                          (IOMUX_PINCM56)
/* Defines for PIN4: GPIOB.24 with pinCMx 52 on package pin 23 */
#define LINE_PIN4_PORT                                                   (GPIOB)
#define LINE_PIN4_PIN                                           (DL_GPIO_PIN_24)
#define LINE_PIN4_IOMUX                                          (IOMUX_PINCM52)
/* Defines for PIN5: GPIOB.20 with pinCMx 48 on package pin 19 */
#define LINE_PIN5_PORT                                                   (GPIOB)
#define LINE_PIN5_PIN                                           (DL_GPIO_PIN_20)
#define LINE_PIN5_IOMUX                                          (IOMUX_PINCM48)
/* Defines for PIN6: GPIOA.22 with pinCMx 47 on package pin 18 */
#define LINE_PIN6_PORT                                                   (GPIOA)
#define LINE_PIN6_PIN                                           (DL_GPIO_PIN_22)
#define LINE_PIN6_IOMUX                                          (IOMUX_PINCM47)
/* Defines for PIN7: GPIOA.14 with pinCMx 36 on package pin 7 */
#define LINE_PIN7_PORT                                                   (GPIOA)
#define LINE_PIN7_PIN                                           (DL_GPIO_PIN_14)
#define LINE_PIN7_IOMUX                                          (IOMUX_PINCM36)
/* Defines for PIN8: GPIOA.16 with pinCMx 38 on package pin 9 */
#define LINE_PIN8_PORT                                                   (GPIOA)
#define LINE_PIN8_PIN                                           (DL_GPIO_PIN_16)
#define LINE_PIN8_IOMUX                                          (IOMUX_PINCM38)
/* Defines for KEY2: GPIOA.7 with pinCMx 14 on package pin 49 */
#define KEY_KEY2_PORT                                                    (GPIOA)
#define KEY_KEY2_PIN                                             (DL_GPIO_PIN_7)
#define KEY_KEY2_IOMUX                                           (IOMUX_PINCM14)
/* Defines for KEY1: GPIOB.23 with pinCMx 51 on package pin 22 */
#define KEY_KEY1_PORT                                                    (GPIOB)
#define KEY_KEY1_PIN                                            (DL_GPIO_PIN_23)
#define KEY_KEY1_IOMUX                                           (IOMUX_PINCM51)
/* Defines for KEY3: GPIOB.27 with pinCMx 58 on package pin 29 */
#define KEY_KEY3_PORT                                                    (GPIOB)
#define KEY_KEY3_PIN                                            (DL_GPIO_PIN_27)
#define KEY_KEY3_IOMUX                                           (IOMUX_PINCM58)
/* Defines for STOP: GPIOB.1 with pinCMx 13 on package pin 48 */
#define KEY_STOP_PORT                                                    (GPIOB)
#define KEY_STOP_PIN                                             (DL_GPIO_PIN_1)
#define KEY_STOP_IOMUX                                           (IOMUX_PINCM13)
/* Port definition for Pin Group ENCODER */
#define ENCODER_PORT                                                     (GPIOA)

/* Defines for E1A: GPIOA.29 with pinCMx 4 on package pin 36 */
// pins affected by this interrupt request:["E1A","E1B","E2A","E2B"]
#define ENCODER_INT_IRQN                                        (GPIOA_INT_IRQn)
#define ENCODER_INT_IIDX                        (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define ENCODER_E1A_IIDX                                    (DL_GPIO_IIDX_DIO29)
#define ENCODER_E1A_PIN                                         (DL_GPIO_PIN_29)
#define ENCODER_E1A_IOMUX                                         (IOMUX_PINCM4)
/* Defines for E1B: GPIOA.13 with pinCMx 35 on package pin 6 */
#define ENCODER_E1B_IIDX                                    (DL_GPIO_IIDX_DIO13)
#define ENCODER_E1B_PIN                                         (DL_GPIO_PIN_13)
#define ENCODER_E1B_IOMUX                                        (IOMUX_PINCM35)
/* Defines for E2A: GPIOA.31 with pinCMx 6 on package pin 39 */
#define ENCODER_E2A_IIDX                                    (DL_GPIO_IIDX_DIO31)
#define ENCODER_E2A_PIN                                         (DL_GPIO_PIN_31)
#define ENCODER_E2A_IOMUX                                         (IOMUX_PINCM6)
/* Defines for E2B: GPIOA.28 with pinCMx 3 on package pin 35 */
#define ENCODER_E2B_IIDX                                    (DL_GPIO_IIDX_DIO28)
#define ENCODER_E2B_PIN                                         (DL_GPIO_PIN_28)
#define ENCODER_E2B_IOMUX                                         (IOMUX_PINCM3)



/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_MOTOR_init(void);
void SYSCFG_DL_SERVO3_init(void);
void SYSCFG_DL_TIMER_0_init(void);
void SYSCFG_DL_I2C_VL53L0X_init(void);
void SYSCFG_DL_MSPMotor_init(void);
void SYSCFG_DL_IMU_init(void);
void SYSCFG_DL_Vision_init(void);
void SYSCFG_DL_user_init(void);
void SYSCFG_DL_NRF_init(void);
void SYSCFG_DL_ADC_BAT_init(void);

void SYSCFG_DL_SYSTICK_init(void);

bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
