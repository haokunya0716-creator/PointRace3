/*
 * SysConfig Configuration Steps:
 *   I2C:
 *     1. Add an I2C module.
 *     2. Name it as "I2C_VL53L0X".
 *     3. Check the box "Enable Controller Mode".
 *     4. Set "Standard Bus Speed" to "Fast Mode (400kHz)". (optional)
 *     5. Set the pins according to your needs.
 *   Optional GPIO:
 *     1. Add XSHUT as output if you want software reset control.
 *     2. GPIO1 is not needed by this polling driver.
 */

#ifndef VL53L0X_H_
#define VL53L0X_H_

#include "ti_msp_dl_config.h"
#include "vl53l0x_api.h"

#if !defined(GPIO_VL53L0X_PIN_VL53L0X_XSHUT_PORT) && defined(GPIO_VL53L0X_PORT)
#define GPIO_VL53L0X_PIN_VL53L0X_XSHUT_PORT GPIO_VL53L0X_PORT 
#endif

#if !defined(GPIO_VL53L0X_PIN_VL53L0X_GPIO1_PORT) && defined(GPIO_VL53L0X_PORT)
#define GPIO_VL53L0X_PIN_VL53L0X_GPIO1_PORT GPIO_VL53L0X_PORT 
#endif

extern VL53L0X_RangingMeasurementData_t RangingMeasurementData;
extern volatile uint8_t VL53L0X_DataReady;
extern volatile uint16_t VL53L0X_Distance_mm;
extern volatile uint8_t VL53L0X_RangeStatus;
extern volatile VL53L0X_Error VL53L0X_LastStatus;

VL53L0X_Error VL53L0X_Init(void);
uint8_t VL53L0X_Poll(void);
void Read_VL53L0X(void);

#endif /* #ifndef VL53L0X_H_ */
