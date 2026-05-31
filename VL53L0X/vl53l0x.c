#include "vl53l0x.h"
#include "vl53l0x_api.h"
#include "clock.h"

typedef enum {
    LONG_RANGE    = 0, /* 远距离模式 */
    HIGH_SPEED    = 1, /* 高速模式 */
    HIGH_ACCURACY = 2, /* 高精度模式 */
} RangingConfig_e;

/* 当前测距模式：
 * HIGH_ACCURACY 精度更高，但一次测距时间更长；
 * HIGH_SPEED 速度更快，但稳定性和精度会低一些；
 * LONG_RANGE 用于更远距离，响应也会相对慢一些。 */
RangingConfig_e RangingConfig = HIGH_SPEED;

/* 旧版单传感器接口使用的设备对象。
 * 三路传感器现在由 Application/app_vl5310x.c 管理。 */
VL53L0X_Dev_t VL53L0XDevs[] = {
    {.Id = 0, .I2cDevAddr = 0x29, .Present = 1},
};

VL53L0X_RangingMeasurementData_t RangingMeasurementData;
volatile uint8_t VL53L0X_DataReady = 0; /* 已经成功读到过距离数据 */
volatile uint16_t VL53L0X_Distance_mm = 0; /* 最新距离，单位 mm */
volatile uint8_t VL53L0X_RangeStatus = 0xFF; /* 0 表示距离有效，非 0 表示测距质量异常 */
volatile VL53L0X_Error VL53L0X_LastStatus = VL53L0X_ERROR_NONE; /* 最近一次 I2C/API 状态 */

/* 初始化一个 VL53L0X 设备。
 *
 * dev：要初始化的设备对象，里面会保存这个传感器的 I2C 地址。
 * i2c_addr_7bit：希望分配给该传感器的新 7 位地址，例如 0x30。
 *
 * 注意：调用这个函数前，应用层必须保证 I2C 总线上只有这个传感器还在默认地址 0x29。
 * 所以三传感器场景需要先用 XSHUT 关闭全部传感器，再逐个上电初始化。 */
VL53L0X_Error VL53L0X_InitDevice(VL53L0X_DEV dev, uint8_t i2c_addr_7bit)
{
    uint16_t Id;
    int32_t status = 0;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;

    FixPoint1616_t signalLimit = (FixPoint1616_t)(0.25 * 65536);
    FixPoint1616_t sigmaLimit = (FixPoint1616_t)(18 * 65536);
    uint32_t timingBudget = 33000;
    uint8_t preRangeVcselPeriod = 14;
    uint8_t finalRangeVcselPeriod = 10;

    if (dev == 0)
        return VL53L0X_ERROR_INVALID_PARAMS;

    /* 每个 VL53L0X 刚上电时都先使用默认地址 0x29。 */
    dev->I2cDevAddr = 0x29;
    dev->Present = 1;

    if (i2c_addr_7bit != 0x29)
    {
        /* ST API 的 SetDeviceAddress 参数使用 8 位地址格式，所以这里左移 1 位。 */
        status += VL53L0X_SetDeviceAddress(dev, i2c_addr_7bit << 1);
        if (status == VL53L0X_ERROR_NONE)
            dev->I2cDevAddr = i2c_addr_7bit;
    }

    /* 读取芯片 ID，并执行 ST 官方初始化流程。 */
    status += VL53L0X_WrByte(dev, 0x88, 0x00);
    status += VL53L0X_RdWord(dev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
    status += VL53L0X_DataInit(dev);
    status += VL53L0X_StaticInit(dev);
    status += VL53L0X_PerformRefSpadManagement(dev, &refSpadCount, &isApertureSpads);
    status += VL53L0X_PerformRefCalibration(dev, &VhvSettings, &PhaseCal);

    status += VL53L0X_SetDeviceMode(dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    status += VL53L0X_SetLimitCheckEnable(dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    status += VL53L0X_SetLimitCheckEnable(dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);

    /* 根据测距模式设置限幅、时序预算和 VCSEL 脉冲周期。 */
    switch(RangingConfig)
    {
        case LONG_RANGE:
            signalLimit = (FixPoint1616_t)(0.1 * 65536);
            sigmaLimit = (FixPoint1616_t)(60 * 65536);
            timingBudget = 33000;
            preRangeVcselPeriod = 18;
            finalRangeVcselPeriod = 14;
            break;
        case HIGH_ACCURACY:
            signalLimit = (FixPoint1616_t)(0.25 * 65536);
            sigmaLimit = (FixPoint1616_t)(18 * 65536);
            timingBudget = 200000;
            preRangeVcselPeriod = 14;
            finalRangeVcselPeriod = 10;
            break;
        case HIGH_SPEED:
            signalLimit = (FixPoint1616_t)(0.25 * 65536);
            sigmaLimit = (FixPoint1616_t)(32 * 65536);
            timingBudget = 20000;
            preRangeVcselPeriod = 14;
            finalRangeVcselPeriod = 10;
            break;
    }

    status += VL53L0X_SetLimitCheckValue(dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, signalLimit);
    status += VL53L0X_SetLimitCheckValue(dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, sigmaLimit);
    status += VL53L0X_SetMeasurementTimingBudgetMicroSeconds(dev, timingBudget);
    status += VL53L0X_SetVcselPulsePeriod(dev, VL53L0X_VCSEL_PERIOD_PRE_RANGE, preRangeVcselPeriod);
    status += VL53L0X_SetVcselPulsePeriod(dev, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, finalRangeVcselPeriod);
    status += VL53L0X_PerformRefCalibration(dev, &VhvSettings, &PhaseCal);

    status += VL53L0X_ClearInterruptMask(dev, 0);

    if (status || (Id != 0xEEAA))
    {
        /* 正常 VL53L0X 的 ID 应该是 0xEEAA，不对就认为通信或器件异常。 */
        VL53L0X_LastStatus = VL53L0X_ERROR_CONTROL_INTERFACE;
        return VL53L0X_LastStatus;
    }

    /* 进入连续测距模式，后续只需要周期性轮询有没有新数据。 */
    status = VL53L0X_StartMeasurement(dev);
    VL53L0X_LastStatus = (VL53L0X_Error)status;
    return VL53L0X_LastStatus;
}

/* 轮询一个 VL53L0X 设备。
 *
 * 返回 1：读到了新的测距结果，measurement 里是本次数据；
 * 返回 0：还没准备好，或者底层通信/API 出错。 */
uint8_t VL53L0X_PollDevice(VL53L0X_DEV dev,
    VL53L0X_RangingMeasurementData_t *measurement)
{
    uint8_t measurementReady = 0;

    if (dev == 0 || measurement == 0)
    {
        VL53L0X_LastStatus = VL53L0X_ERROR_INVALID_PARAMS;
        return 0;
    }

    VL53L0X_LastStatus = VL53L0X_GetMeasurementDataReady(dev, &measurementReady);
    if (VL53L0X_LastStatus != VL53L0X_ERROR_NONE)
    {
        VL53L0X_DataReady = 0;
        return 0;
    }
    if (!measurementReady)
        return 0;

    /* 有新结果时读取距离和 RangeStatus。 */
    VL53L0X_LastStatus = VL53L0X_GetRangingMeasurementData(dev, measurement);
    if (VL53L0X_LastStatus != VL53L0X_ERROR_NONE)
    {
        VL53L0X_DataReady = 0;
        return 0;
    }

    /* 连续测距模式下，读完结果后要清中断标志，传感器才会继续准备下一次结果。 */
    VL53L0X_LastStatus = VL53L0X_ClearInterruptMask(dev, 0);
    return (VL53L0X_LastStatus == VL53L0X_ERROR_NONE);
}

/* 兼容旧代码：初始化单个默认地址传感器。 */
VL53L0X_Error VL53L0X_Init(void)
{
    return VL53L0X_InitDevice(&VL53L0XDevs[0], 0x29);
}

/* 兼容旧代码：轮询单个传感器，并更新旧的全局变量。 */
uint8_t VL53L0X_Poll(void)
{
    uint8_t ok = VL53L0X_PollDevice(&VL53L0XDevs[0], &RangingMeasurementData);

    if (ok)
    {
        VL53L0X_Distance_mm = RangingMeasurementData.RangeMilliMeter;
        VL53L0X_RangeStatus = RangingMeasurementData.RangeStatus;
        VL53L0X_DataReady = 1;
    }

    return ok;
}

void Read_VL53L0X(void)
{
    (void)VL53L0X_Poll();
}
