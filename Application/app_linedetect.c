#include "app_linedetect.h"

volatile uint8_t line_detect[8] = {0};//存放黑白检测结果的数组
//黑线为1，白线为0

/**
 * @brief 扫描 8 路灰度循迹传感器并更新黑白检测结果。
 * @property 黑线低电平有效，line_detect[i] 为 1 表示黑线，为 0 表示白线。
 */
void Line_detect(void){
    line_detect[0] = (DL_GPIO_readPins(LINE_PIN1_PORT, LINE_PIN1_PIN) == 0U) ? 1U : 0U;
    line_detect[1] = (DL_GPIO_readPins(LINE_PIN2_PORT, LINE_PIN2_PIN) == 0U) ? 1U : 0U;
    line_detect[2] = (DL_GPIO_readPins(LINE_PIN3_PORT, LINE_PIN3_PIN) == 0U) ? 1U : 0U;
    line_detect[3] = (DL_GPIO_readPins(LINE_PIN4_PORT, LINE_PIN4_PIN) == 0U) ? 1U : 0U;
    line_detect[4] = (DL_GPIO_readPins(LINE_PIN5_PORT, LINE_PIN5_PIN) == 0U) ? 1U : 0U;
    line_detect[5] = (DL_GPIO_readPins(LINE_PIN6_PORT, LINE_PIN6_PIN) == 0U) ? 1U : 0U;
    line_detect[6] = (DL_GPIO_readPins(LINE_PIN7_PORT, LINE_PIN7_PIN) == 0U) ? 1U : 0U;
    line_detect[7] = (DL_GPIO_readPins(LINE_PIN8_PORT, LINE_PIN8_PIN) == 0U) ? 1U : 0U;
}
