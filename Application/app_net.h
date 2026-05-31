#ifndef APP_NET_H_
#define APP_NET_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

#define NET_W              4      // 地图横向 4 个区块，x 从左到右为 0~3
#define NET_H              4      // 地图纵向 4 个区块，y 从上到下为 0~3
#define NET_CELL_CM        45.0f  // 每个方格边长，单位 cm，实车可在 43~45cm 间微调
#define NET_ENTER_CM       72.0f  // 从准备区经 A 口进入第一个格子中心的距离，单位 cm
#define NET_EXIT_CM        70.0f  // 从 C 口所在格子中心驶出场地的距离，单位 cm
#define NET_POS_TOL_CM     2.0f   // 位置闭环允许误差，避免电机在最后几厘米反复抖动
#define NET_BLOCK_MM       280    // 在格子中心探测前方挡板的距离阈值，单位 mm，需要实测调整

typedef enum {
    NET_IDLE = 0,  // 未启动
    NET_ENTER,     // 从 A 口进入起始格子中心
    NET_PLAN,      // 根据已知挡板信息重新 BFS 规划路线
    NET_STEP,      // 取出路径中的下一步方向
    NET_CHECK,     // 在格子中心检测下一条边是否有挡板
    NET_GO,        // 沿当前方向走一格
    NET_EXIT_TURN, // 16 格遍历完成后，转向 C 口方向
    NET_EXIT_GO,   // 从 C 口驶出场地
    NET_DONE,      // 任务完成
    NET_ERROR      // 无路可走或规划失败
} NetState_t;

extern volatile NetState_t net_state;    // 当前第二问任务状态
extern volatile uint8_t net_x;           // 当前所在格子的 x 坐标
extern volatile uint8_t net_y;           // 当前所在格子的 y 坐标
extern volatile uint8_t net_dir;         // 当前车头方向，0 上、1 右、2 下、3 左
extern volatile uint8_t net_visited_num; // 已遍历过的区块数量，目标为 16

/**
 * @brief 初始化第二问遍历任务。
 * @param 无
 * @return 无
 */
void App_Net_Init(void);

/**
 * @brief 启动第二问遍历任务。
 * @param 无
 * @return 无
 */
void App_Net_Start(void);

/**
 * @brief 第二问遍历任务进程函数，需要在主循环中高频调用。
 * @param 无
 * @return 无
 */
void App_Net_Proc(void);

/**
 * @brief 获取某个格子是否已经遍历。
 * @param x 横坐标，0~3。
 * @param y 纵坐标，0~3。
 * @return 1 表示已遍历，0 表示未遍历。
 */
uint8_t App_Net_IsVisited(uint8_t x, uint8_t y);

#endif /* APP_NET_H_ */
