/*
 * VL53L0X 基础驱动接口。
 *
 * 这一层只负责“某一个 VL53L0X 怎么初始化、怎么轮询读取距离”。
 * 三个传感器的 XSHUT 上电顺序、地址分配、前/左/右数组管理，
 * 放在 Application/app_vl5310x.c 里。
 *
 * SysConfig 至少需要配置 I2C_VL53L0X：
 *   1. 添加 I2C 模块；
 *   2. 名字设为 I2C_VL53L0X；
 *   3. 使能 Controller Mode；
 *   4. 建议 I2C 速度设为 Fast Mode 400kHz；
 *   5. SDA/SCL 按 PCB 接线配置。
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

/* 初始化指定 dev，并在初始化开始时把默认地址 0x29 改成 i2c_addr_7bit。 */
VL53L0X_Error VL53L0X_InitDevice(VL53L0X_DEV dev, uint8_t i2c_addr_7bit);

/* 轮询指定 dev；如果有新测距结果，就写入 measurement 并返回 1。 */
uint8_t VL53L0X_PollDevice(VL53L0X_DEV dev,
    VL53L0X_RangingMeasurementData_t *measurement);

/* 兼容旧代码的单传感器接口，默认只操作 VL53L0XDevs[0]。 */
VL53L0X_Error VL53L0X_Init(void);
uint8_t VL53L0X_Poll(void);
void Read_VL53L0X(void);

#endif /* #ifndef VL53L0X_H_ */
