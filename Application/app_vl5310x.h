#ifndef APP_VL5310X_H_
#define APP_VL5310X_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"
#include "vl53l0x.h"

#define VL5310X_FRONT_OBSTACLE_MM  250 // 前方避障阈值，单位 mm
#define VL5310X_SIDE_OBSTACLE_MM   180 // 左右避障阈值，单位 mm

/**
 * @brief 三个 VL53L0X 的逻辑编号。
 * @note VL5310X_COUNT 是传感器数量，用来定义数组长度，非常置方便且简洁。
 */
typedef enum {
    VL5310X_FRONT = 0, // 前方传感器，物理引脚为 XSHUT2
    VL5310X_LEFT = 1,      // 左侧传感器，物理引脚为 XSHUT1
    VL5310X_RIGHT = 2,     // 右侧传感器，物理引脚为 XSHUT3
    VL5310X_COUNT = 3,   // 传感器数量：前、左、右，一共 3 个
	} VL5310X_SensorId_t; //激光测距的枚举（前，左，右分别对应0，1，2。传感器总数为3）

extern volatile uint16_t VL5310X_Distance_mm[VL5310X_COUNT]; // 三路最新距离，单位 mm


extern volatile uint8_t vl5310x_front_obstacle_flag; // 前方障碍标志，1 表示距离小于 VL5310X_FRONT_OBSTACLE_MM
extern volatile uint8_t vl5310x_left_obstacle_flag;  // 左侧障碍标志，1 表示距离小于 VL5310X_SIDE_OBSTACLE_MM
extern volatile uint8_t vl5310x_right_obstacle_flag; // 右侧障碍标志，1 表示距离小于 VL5310X_SIDE_OBSTACLE_MM

/**
 * @brief 初始化三路 VL53L0X。
 * @param 无
 * @return 无
 * @note 会通过 XSHUT 逐个上电，并把默认地址 0x29 改成 0x30、0x31、0x32。
 */
void App_VL5310X_Init(void);

/**
 * @brief 轮询三路 VL53L0X，并更新距离和避障标志。
 * @param 无
 * @return 无
 * @note 建议在主循环里 50ms 左右调用一次。
 */
void App_VL5310X_Proc(void);

/**
 * @brief 获取某一路最新距离。
 * @param id 传感器编号，可选 VL5310X_FRONT、VL5310X_LEFT、VL5310X_RIGHT。
 * @return 最新距离，单位 mm；id 不合法时返回 0。
 */
uint16_t App_VL5310X_GetDistance(VL5310X_SensorId_t id);

#endif /* APP_VL5310X_H_ */
