#include "app_linedetect.h"
#include "clock.h"

volatile uint8_t line_raw[LINE_SENSOR_NUM] = {0}; // X1~X8 原始读数
volatile uint8_t line_black_flag = 0;             // 是否检测到任意黑线
volatile uint8_t line_cross_flag = 0;             // 是否检测到横线/边界线
volatile uint8_t line_black_count = 0;            // 检测到黑线的通道数量
volatile uint8_t line_black_mask = 0;             // 黑线通道位图

/**
 * @brief 设置 AD0/AD1/AD2，选择当前要读取的灰度通道。
 * @param ch 通道编号，0~7 对应 X1~X8。
 * @return 无
 */
static void SetCh(uint8_t ch)
{
    if (ch & 0x01)
        DL_GPIO_setPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD0_X1_PIN);
    else
        DL_GPIO_clearPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD0_X1_PIN);

    if (ch & 0x02)
        DL_GPIO_setPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD1_X2_PIN);
    else
        DL_GPIO_clearPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD1_X2_PIN);

    if (ch & 0x04)
        DL_GPIO_setPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD2_X3_PIN);
    else
        DL_GPIO_clearPins(LINE_FOLLOW_PORT, LINE_FOLLOW_AD2_X3_PIN);
}

/**
 * @brief 读取当前选中通道的 OUT 电平。
 * @param 无
 * @return 1 表示 OUT 为高电平，0 表示 OUT 为低电平。
 */
static uint8_t ReadOut(void)
{
    return (DL_GPIO_readPins(LINE_FOLLOW_PORT, LINE_FOLLOW_OUT_X4_PIN) != 0) ? 1 : 0;
}

/**
 * @brief 扫描八路灰度传感器，并更新黑线检测状态。
 * @param 无
 * @return 无
 */
void App_LineDetect_Proc(void)
{
    uint8_t mask = 0;  // 本次扫描得到的黑线位图
    uint8_t count = 0; // 本次扫描检测到黑线的通道数量

    for (uint8_t i = 0; i < LINE_SENSOR_NUM; i++)
    {
        SetCh(i);
        mspm0_delay_ms(1);

        line_raw[i] = ReadOut();

        if (line_raw[i] == LINE_SENSOR_ACTIVE_LEVEL)
        {
            mask |= (uint8_t)(1U << i);
            count++;
        }
    }

    line_black_mask = mask;
    line_black_count = count;
    line_black_flag = (count > 0) ? 1 : 0;
    line_cross_flag = (count >= 2) ? 1 : 0; // 如果实车太灵敏，可以改成 count >= 3
}
