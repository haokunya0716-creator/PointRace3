
#ifndef LINE_DETECT_H_
#define LINE_DETECT_H_

#include <stdint.h>
#include "ti_msp_dl_config.h"

extern volatile uint8_t line_detect[8];//黑线为1，白线为0

/**
 * @brief 扫描 8 路灰度循迹传感器并更新黑白检测结果。
 * @property 黑线低电平有效，line_detect[i] 为 1 表示黑线，为 0 表示白线。
 */
void Line_detect(void);

#endif /* LINE_DETECT_H_ */
