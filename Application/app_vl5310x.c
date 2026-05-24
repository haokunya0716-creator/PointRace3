#include "app_vl5310x.h"
#include "clock.h"

/**
 * @brief 单个 VL53L0X 的 XSHUT 引脚信息。
 * @note 只在本文件内部使用，外部代码不需要关心具体引脚。
 */
typedef struct {
    GPIO_Regs *port; // XSHUT 所在 GPIO 端口
    uint32_t pin;    // XSHUT 引脚
    uint32_t iomux;  // XSHUT 对应 IOMUX
} XshutPin_t;

/**
 * @brief 三个 XSHUT 引脚和传感器逻辑方向的对应关系。
 * @note PCB/SysConfig 关系为：XSHUT1=左，XSHUT2=前，XSHUT3=右。
 */
static const XshutPin_t xshut_pin[VL5310X_COUNT] = {
    [VL5310X_FRONT] = {XSHUT_XSHUT2_PORT, XSHUT_XSHUT2_PIN, XSHUT_XSHUT2_IOMUX},
    [VL5310X_LEFT]  = {XSHUT_XSHUT1_PORT, XSHUT_XSHUT1_PIN, XSHUT_XSHUT1_IOMUX},
    [VL5310X_RIGHT] = {XSHUT_XSHUT3_PORT, XSHUT_XSHUT3_PIN, XSHUT_XSHUT3_IOMUX},
};

/**
 * @brief 三个传感器最终使用的 I2C 地址。
 * @note 初始化时会从默认地址 0x29 改成这里的地址。
 */
static const uint8_t addr[VL5310X_COUNT] = {
    [VL5310X_FRONT] = 0x30,
    [VL5310X_LEFT]  = 0x31,
    [VL5310X_RIGHT] = 0x32,
};

static VL53L0X_Dev_t dev[VL5310X_COUNT] = {
    [VL5310X_FRONT] = {.Id = VL5310X_FRONT, .I2cDevAddr = 0x29, .Present = 1},
    [VL5310X_LEFT]  = {.Id = VL5310X_LEFT,  .I2cDevAddr = 0x29, .Present = 1},
    [VL5310X_RIGHT] = {.Id = VL5310X_RIGHT, .I2cDevAddr = 0x29, .Present = 1},
}; // VL53L0X 官方 API 使用的设备对象

static VL53L0X_RangingMeasurementData_t meas[VL5310X_COUNT]; // 官方 API 返回的测距数据

volatile uint16_t VL5310X_Distance_mm[VL5310X_COUNT] = {0}; // 三路最新距离
volatile uint8_t VL5310X_RangeStatus[VL5310X_COUNT] = {0xFF, 0xFF, 0xFF}; // 三路测距质量
volatile uint8_t VL5310X_DataReady[VL5310X_COUNT] = {0}; // 三路是否已经读到过距离
volatile uint8_t VL5310X_Online[VL5310X_COUNT] = {0}; // 三路是否初始化成功
volatile VL53L0X_Error VL5310X_LastError[VL5310X_COUNT] = {
    VL53L0X_ERROR_NONE,
    VL53L0X_ERROR_NONE,
    VL53L0X_ERROR_NONE,
}; // 三路最近一次 API/I2C 状态

volatile uint8_t vl5310x_front_obstacle_flag = 0; // 前方障碍标志
volatile uint8_t vl5310x_left_obstacle_flag = 0;  // 左侧障碍标志
volatile uint8_t vl5310x_right_obstacle_flag = 0; // 右侧障碍标志

/**
 * @brief 控制某一路 XSHUT 电平。
 * @param id 传感器编号。
 * @param high 1 表示拉高上电，0 表示拉低关机。
 * @return 无
 */
static void XshutWrite(VL5310X_SensorId_t id, uint8_t high)
{
    if (high)
        DL_GPIO_setPins(xshut_pin[id].port, xshut_pin[id].pin);
    else
        DL_GPIO_clearPins(xshut_pin[id].port, xshut_pin[id].pin);
}

/**
 * @brief 初始化 XSHUT 引脚，并先全部拉低。
 * @param 无
 * @return 无
 * @note 全部拉低后，I2C 总线上暂时没有默认地址 0x29 的设备。
 */
static void XshutInit(void)
{
    for (uint8_t i = 0; i < VL5310X_COUNT; i++)
    {
        DL_GPIO_initDigitalOutput(xshut_pin[i].iomux);
        DL_GPIO_clearPins(xshut_pin[i].port, xshut_pin[i].pin);
        DL_GPIO_enableOutput(xshut_pin[i].port, xshut_pin[i].pin);
    }
}

/**
 * @brief 判断某一路距离是否有效。
 * @param id 传感器编号。
 * @return 1 表示有效，0 表示无效。
 */
static uint8_t IsValid(VL5310X_SensorId_t id)
{
    return (VL5310X_Online[id] &&
            VL5310X_DataReady[id] &&
            VL5310X_LastError[id] == VL53L0X_ERROR_NONE &&
            VL5310X_RangeStatus[id] == 0);
}

/**
 * @brief 判断某一路是否检测到障碍。
 * @param id 传感器编号。
 * @param limit_mm 障碍判断阈值，单位 mm。
 * @return 1 表示有障碍，0 表示无障碍或数据无效。
 */
static uint8_t IsObstacle(VL5310X_SensorId_t id, uint16_t limit_mm)
{
    return (IsValid(id) && VL5310X_Distance_mm[id] <= limit_mm) ? 1 : 0;
}

/**
 * @brief 初始化三路 VL53L0X。
 * @param 无
 * @return 无
 */
void App_VL5310X_Init(void)
{
    XshutInit();
    mspm0_delay_ms(5);

    for (uint8_t i = 0; i < VL5310X_COUNT; i++)
    {
			XshutWrite((VL5310X_SensorId_t)i, 1);//先单独把这一路拉高，改地址（改地址时，这个I2C上只能由一路地址为0x29的有效外设，否则改不了）
        mspm0_delay_ms(5);

        VL5310X_LastError[i] = VL53L0X_InitDevice(&dev[i], addr[i]);
        VL5310X_Online[i] = (VL5310X_LastError[i] == VL53L0X_ERROR_NONE) ? 1 : 0;
        VL5310X_DataReady[i] = 0;
        VL5310X_RangeStatus[i] = 0xFF;
    }
}

/**
 * @brief 轮询三路 VL53L0X，并更新距离和避障标志。
 * @param 无
 * @return 无
 */
void App_VL5310X_Proc(void)
{
    for (uint8_t i = 0; i < VL5310X_COUNT; i++)
    {
			//如果已经初始化成功的话,这个数组对应的值会是1.
			//如果未初始化成功的话就会直接跳过这个传感器对应的循环
        if (!VL5310X_Online[i])
				{
            continue;
				}

        if (VL53L0X_PollDevice(&dev[i], &meas[i]))
        {
            VL5310X_Distance_mm[i] = meas[i].RangeMilliMeter;
            VL5310X_RangeStatus[i] = meas[i].RangeStatus;
            VL5310X_DataReady[i] = 1;
            VL5310X_LastError[i] = VL53L0X_ERROR_NONE;
        }
        else
        {
            VL5310X_LastError[i] = VL53L0X_LastStatus;
        }
    }

    vl5310x_front_obstacle_flag = IsObstacle(VL5310X_FRONT, VL5310X_FRONT_OBSTACLE_MM);
    vl5310x_left_obstacle_flag = IsObstacle(VL5310X_LEFT, VL5310X_SIDE_OBSTACLE_MM);
    vl5310x_right_obstacle_flag = IsObstacle(VL5310X_RIGHT, VL5310X_SIDE_OBSTACLE_MM);
}

/**
 * @brief 获取某一路最新距离。
 * @param id 传感器编号。
 * @return 最新距离，单位 mm；id 不合法时返回 0。
 */
uint16_t App_VL5310X_GetDistance(VL5310X_SensorId_t id)
{
    if (id >= VL5310X_COUNT)
        return 0;

    return VL5310X_Distance_mm[id];
}
