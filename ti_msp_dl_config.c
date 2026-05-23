/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerG_backupConfig gPWM_0Backup;
DL_TimerA_backupConfig gSERVO_1Backup;
DL_TimerA_backupConfig gTIMER_0Backup;
DL_UART_Main_backupConfig gMSPMotorBackup;
DL_SPI_backupConfig gSPI_LCDBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_PWM_0_init();
    SYSCFG_DL_SERVO_1_init();
    SYSCFG_DL_TIMER_0_init();
    SYSCFG_DL_I2C_VL53L0X_init();
    SYSCFG_DL_user_init();
    SYSCFG_DL_MSPMotor_init();
    SYSCFG_DL_IMU_init();
    SYSCFG_DL_Vision_init();
    SYSCFG_DL_SPI_LCD_init();
    SYSCFG_DL_SYSTICK_init();
    /* Ensure backup structures have no valid state */
	gPWM_0Backup.backupRdy 	= false;
	gSERVO_1Backup.backupRdy 	= false;
	gTIMER_0Backup.backupRdy 	= false;
	gMSPMotorBackup.backupRdy 	= false;
	gSPI_LCDBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_saveConfiguration(PWM_0_INST, &gPWM_0Backup);
	retStatus &= DL_TimerA_saveConfiguration(SERVO_1_INST, &gSERVO_1Backup);
	retStatus &= DL_TimerA_saveConfiguration(TIMER_0_INST, &gTIMER_0Backup);
	retStatus &= DL_UART_Main_saveConfiguration(MSPMotor_INST, &gMSPMotorBackup);
	retStatus &= DL_SPI_saveConfiguration(SPI_LCD_INST, &gSPI_LCDBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerG_restoreConfiguration(PWM_0_INST, &gPWM_0Backup, false);
	retStatus &= DL_TimerA_restoreConfiguration(SERVO_1_INST, &gSERVO_1Backup, false);
	retStatus &= DL_TimerA_restoreConfiguration(TIMER_0_INST, &gTIMER_0Backup, false);
	retStatus &= DL_UART_Main_restoreConfiguration(MSPMotor_INST, &gMSPMotorBackup);
	retStatus &= DL_SPI_restoreConfiguration(SPI_LCD_INST, &gSPI_LCDBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerG_reset(PWM_0_INST);
    DL_TimerA_reset(SERVO_1_INST);
    DL_TimerA_reset(TIMER_0_INST);
    DL_I2C_reset(I2C_VL53L0X_INST);
    DL_UART_Main_reset(user_INST);
    DL_UART_Main_reset(MSPMotor_INST);
    DL_UART_Main_reset(IMU_INST);
    DL_UART_Main_reset(Vision_INST);
    DL_SPI_reset(SPI_LCD_INST);


    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerG_enablePower(PWM_0_INST);
    DL_TimerA_enablePower(SERVO_1_INST);
    DL_TimerA_enablePower(TIMER_0_INST);
    DL_I2C_enablePower(I2C_VL53L0X_INST);
    DL_UART_Main_enablePower(user_INST);
    DL_UART_Main_enablePower(MSPMotor_INST);
    DL_UART_Main_enablePower(IMU_INST);
    DL_UART_Main_enablePower(Vision_INST);
    DL_SPI_enablePower(SPI_LCD_INST);

    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_0_C0_IOMUX,GPIO_PWM_0_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_0_C0_PORT, GPIO_PWM_0_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM_0_C1_IOMUX,GPIO_PWM_0_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_PWM_0_C1_PORT, GPIO_PWM_0_C1_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_SERVO_1_C0_IOMUX,GPIO_SERVO_1_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_SERVO_1_C0_PORT, GPIO_SERVO_1_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_SERVO_1_C1_IOMUX,GPIO_SERVO_1_C1_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_SERVO_1_C1_PORT, GPIO_SERVO_1_C1_PIN);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_VL53L0X_IOMUX_SDA,
        GPIO_I2C_VL53L0X_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_VL53L0X_IOMUX_SCL,
        GPIO_I2C_VL53L0X_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_VL53L0X_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_VL53L0X_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_user_IOMUX_TX, GPIO_user_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_user_IOMUX_RX, GPIO_user_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_MSPMotor_IOMUX_TX, GPIO_MSPMotor_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_MSPMotor_IOMUX_RX, GPIO_MSPMotor_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_IMU_IOMUX_TX, GPIO_IMU_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_IMU_IOMUX_RX, GPIO_IMU_IOMUX_RX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_Vision_IOMUX_TX, GPIO_Vision_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_Vision_IOMUX_RX, GPIO_Vision_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_LCD_IOMUX_SCLK, GPIO_SPI_LCD_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_LCD_IOMUX_PICO, GPIO_SPI_LCD_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_LCD_IOMUX_POCI, GPIO_SPI_LCD_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(XSHUT_XSHUT1_IOMUX);

    DL_GPIO_initDigitalOutput(XSHUT_XSHUT2_IOMUX);

    DL_GPIO_initDigitalOutput(XSHUT_XSHUT3_IOMUX);

    DL_GPIO_initDigitalOutput(LINE_FOLLOW_AD0_X1_IOMUX);

    DL_GPIO_initDigitalOutput(LINE_FOLLOW_AD1_X2_IOMUX);

    DL_GPIO_initDigitalOutput(LINE_FOLLOW_AD2_X3_IOMUX);

    DL_GPIO_initDigitalInputFeatures(LINE_FOLLOW_OUT_X4_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(LCD_RES_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_DC_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_CS_IOMUX);

    DL_GPIO_initDigitalOutput(LCD_BLK_IOMUX);

    DL_GPIO_clearPins(GPIOA, XSHUT_XSHUT1_PIN |
		LINE_FOLLOW_AD0_X1_PIN |
		LINE_FOLLOW_AD1_X2_PIN |
		LINE_FOLLOW_AD2_X3_PIN);
    DL_GPIO_enableOutput(GPIOA, XSHUT_XSHUT1_PIN |
		LINE_FOLLOW_AD0_X1_PIN |
		LINE_FOLLOW_AD1_X2_PIN |
		LINE_FOLLOW_AD2_X3_PIN);
    DL_GPIO_clearPins(GPIOB, XSHUT_XSHUT2_PIN |
		XSHUT_XSHUT3_PIN |
		LCD_RES_PIN |
		LCD_DC_PIN |
		LCD_CS_PIN |
		LCD_BLK_PIN);
    DL_GPIO_enableOutput(GPIOB, XSHUT_XSHUT2_PIN |
		XSHUT_XSHUT3_PIN |
		LCD_RES_PIN |
		LCD_DC_PIN |
		LCD_CS_PIN |
		LCD_BLK_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq              = DL_SYSCTL_SYSPLL_INPUT_FREQ_16_32_MHZ,
	.rDivClk2x              = 3,
	.rDivClk1               = 0,
	.rDivClk0               = 0,
	.enableCLK2x            = DL_SYSCTL_SYSPLL_CLK2X_ENABLE,
	.enableCLK1             = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
	.enableCLK0             = DL_SYSCTL_SYSPLL_CLK0_DISABLE,
	.sysPLLMCLK             = DL_SYSCTL_SYSPLL_MCLK_CLK2X,
	.sysPLLRef              = DL_SYSCTL_SYSPLL_REF_SYSOSC,
	.qDiv                   = 9,
	.pDiv                   = DL_SYSCTL_SYSPLL_PDIV_2
};
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_enableMFCLK();
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

}


/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   500000 Hz = 80000000 Hz / (1 * (159 + 1))
 */
static const DL_TimerG_ClockConfig gPWM_0ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 159U
};

static const DL_TimerG_PWMConfig gPWM_0Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN_UP,
    .period = 10000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_PWM_0_init(void) {

    DL_TimerG_setClockConfig(
        PWM_0_INST, (DL_TimerG_ClockConfig *) &gPWM_0ClockConfig);

    DL_TimerG_initPWMMode(
        PWM_0_INST, (DL_TimerG_PWMConfig *) &gPWM_0Config);

    DL_TimerG_setCaptureCompareOutCtl(PWM_0_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_0_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_0_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 5000, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareOutCtl(PWM_0_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERG_CAPTURE_COMPARE_1_INDEX);

    DL_TimerG_setCaptCompUpdateMethod(PWM_0_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERG_CAPTURE_COMPARE_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, DL_TIMER_CC_1_INDEX);

    DL_TimerG_enableClock(PWM_0_INST);


    
    DL_TimerG_setCCPDirection(PWM_0_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}
/*
 * Timer clock configuration to be sourced by  / 1 (80000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   500000 Hz = 80000000 Hz / (1 * (159 + 1))
 */
static const DL_TimerA_ClockConfig gSERVO_1ClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 159U
};

static const DL_TimerA_PWMConfig gSERVO_1Config = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 10000,
    .isTimerWithFourCC = false,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_SERVO_1_init(void) {

    DL_TimerA_setClockConfig(
        SERVO_1_INST, (DL_TimerA_ClockConfig *) &gSERVO_1ClockConfig);

    DL_TimerA_initPWMMode(
        SERVO_1_INST, (DL_TimerA_PWMConfig *) &gSERVO_1Config);

    DL_TimerA_setCaptureCompareOutCtl(SERVO_1_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(SERVO_1_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(SERVO_1_INST, 10000, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(SERVO_1_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_1_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(SERVO_1_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_1_INDEX);
    DL_TimerA_setCaptureCompareValue(SERVO_1_INST, 10000, DL_TIMER_CC_1_INDEX);

    DL_TimerA_enableClock(SERVO_1_INST);


    
    DL_TimerA_setCCPDirection(SERVO_1_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT );


}



/*
 * Timer clock configuration to be sourced by MFCLK /  (4000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   4000000 Hz = 4000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gTIMER_0ClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_MFCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale    = 0U,
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * TIMER_0_INST_LOAD_VALUE = (1ms * 4000000 Hz) - 1
 */
static const DL_TimerA_TimerConfig gTIMER_0TimerConfig = {
    .period     = TIMER_0_INST_LOAD_VALUE,
    .timerMode  = DL_TIMER_TIMER_MODE_PERIODIC_UP,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_TIMER_0_init(void) {

    DL_TimerA_setClockConfig(TIMER_0_INST,
        (DL_TimerA_ClockConfig *) &gTIMER_0ClockConfig);

    DL_TimerA_initTimerMode(TIMER_0_INST,
        (DL_TimerA_TimerConfig *) &gTIMER_0TimerConfig);
    DL_TimerA_enableInterrupt(TIMER_0_INST , DL_TIMERA_INTERRUPT_LOAD_EVENT |
		DL_TIMERA_INTERRUPT_ZERO_EVENT);
	NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 2);
    DL_TimerA_enableClock(TIMER_0_INST);





}


static const DL_I2C_ClockConfig gI2C_VL53L0XClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_VL53L0X_init(void) {

    DL_I2C_setClockConfig(I2C_VL53L0X_INST,
        (DL_I2C_ClockConfig *) &gI2C_VL53L0XClockConfig);
    DL_I2C_setAnalogGlitchFilterPulseWidth(I2C_VL53L0X_INST,
        DL_I2C_ANALOG_GLITCH_FILTER_WIDTH_50NS);
    DL_I2C_enableAnalogGlitchFilter(I2C_VL53L0X_INST);




}


static const DL_UART_Main_ClockConfig guserClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_MFCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config guserConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_user_init(void)
{
    DL_UART_Main_setClockConfig(user_INST, (DL_UART_Main_ClockConfig *) &guserClockConfig);

    DL_UART_Main_init(user_INST, (DL_UART_Main_Config *) &guserConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115107.91
     */
    DL_UART_Main_setOversampling(user_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(user_INST, user_IBRD_4_MHZ_115200_BAUD, user_FBRD_4_MHZ_115200_BAUD);



    DL_UART_Main_enable(user_INST);
}

static const DL_UART_Main_ClockConfig gMSPMotorClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_MFCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gMSPMotorConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_MSPMotor_init(void)
{
    DL_UART_Main_setClockConfig(MSPMotor_INST, (DL_UART_Main_ClockConfig *) &gMSPMotorClockConfig);

    DL_UART_Main_init(MSPMotor_INST, (DL_UART_Main_Config *) &gMSPMotorConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115107.91
     */
    DL_UART_Main_setOversampling(MSPMotor_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(MSPMotor_INST, MSPMotor_IBRD_4_MHZ_115200_BAUD, MSPMotor_FBRD_4_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(MSPMotor_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(MSPMotor_INST_INT_IRQN, 0);


    DL_UART_Main_enable(MSPMotor_INST);
}

static const DL_UART_Main_ClockConfig gIMUClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gIMUConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_IMU_init(void)
{
    DL_UART_Main_setClockConfig(IMU_INST, (DL_UART_Main_ClockConfig *) &gIMUClockConfig);

    DL_UART_Main_init(IMU_INST, (DL_UART_Main_Config *) &gIMUConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(IMU_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(IMU_INST, IMU_IBRD_40_MHZ_115200_BAUD, IMU_FBRD_40_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(IMU_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    /* Setting the Interrupt Priority */
    NVIC_SetPriority(IMU_INST_INT_IRQN, 1);


    DL_UART_Main_enable(IMU_INST);
}

static const DL_UART_Main_ClockConfig gVisionClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gVisionConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_Vision_init(void)
{
    DL_UART_Main_setClockConfig(Vision_INST, (DL_UART_Main_ClockConfig *) &gVisionClockConfig);

    DL_UART_Main_init(Vision_INST, (DL_UART_Main_Config *) &gVisionConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(Vision_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(Vision_INST, Vision_IBRD_40_MHZ_115200_BAUD, Vision_FBRD_40_MHZ_115200_BAUD);



    DL_UART_Main_enable(Vision_INST);
}

static const DL_SPI_Config gSPI_LCD_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gSPI_LCD_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_LCD_init(void) {
    DL_SPI_setClockConfig(SPI_LCD_INST, (DL_SPI_ClockConfig *) &gSPI_LCD_clockConfig);

    DL_SPI_init(SPI_LCD_INST, (DL_SPI_Config *) &gSPI_LCD_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     20000000 = (80000000)/((1 + 1) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(SPI_LCD_INST, 1);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(SPI_LCD_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(SPI_LCD_INST);
}

SYSCONFIG_WEAK void SYSCFG_DL_SYSTICK_init(void)
{
    /*
     * Initializes the SysTick period to 1.00 ms,
     * enables the interrupt, and starts the SysTick Timer
     */
    DL_SYSTICK_config(80000);
}

