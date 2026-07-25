#include "app_linedetect.h"
#include "pid.h"
#include "app_speed.h"
#include "motor.h"
#include "clock.h"

#define LINE_LOST_CONFIRM_MS 400U

volatile uint8_t line_detect[8] = {0};//存放黑白检测结果的数组
//黑线为1，白线为0

static PID_TypeDef line_follow_pid;
static uint8_t line_lost_timing = 0U;
static uint32_t line_lost_start_ms = 0U;

float line_flag = 1;//循迹标志位 1 -> 有线
										//					 0 -> 无线

float line_data = 0.0f;//检测到所有黑带位置值的总和
float line_geshu = 0.0f;//检测到黑带的个数

float line_value = 3.5f;

float last_line_value = 3.5f;//上一次的黑带位置平均值

float speed_base = 40.0f;//基础速度，单位 cm/s

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

/**
 * @brief 初始化循迹 PID 控制器。
 * @property 循迹位置使用 0~7 权重，目标中心值为 3.5，PID 输出限幅为 -60~60。
 */
void App_Line_Follow_Init(void)
{
    PID_Init(&line_follow_pid, 0.95f, 0.85f, 0.0f);
    PID_LimitConfig(&line_follow_pid, 60.0f, -60.0f);
    PID_ChangeSP(&line_follow_pid, 3.5f);

    line_data = 0.0f;
    line_geshu = 0.0f;
    line_value = 3.5f;
    last_line_value = 3.5f;
    line_flag = 1;
    line_lost_timing = 0U;
    line_lost_start_ms = 0U;
}

/**
 * @brief 复位循迹 PID 控制器和循迹位置缓存。
 * @property 复位后默认循迹位置回到中心值 3.5。
 */
void App_Line_Follow_Reset(void){
    PID_Reset(&line_follow_pid);

    line_data = 0.0f;
    line_geshu = 0.0f;
    line_value = 3.5f;
    last_line_value = 3.5f;
    line_flag = 1;
    line_lost_timing = 0U;
    line_lost_start_ms = 0U;
}

/**
 * @brief 刷新灰度检测并计算当前黑线位置平均值。
 * @property 每路权重为数组下标 0~7；未检测到黑线时沿用 last_line_value，避免除以 0。
 */
void Line_Follow_UpdateData(void){
    uint8_t i = 0;
    uint32_t now_ms = 0U;

    line_data = 0.0f;
    line_geshu = 0.0f;

    Line_detect();

    for(i = 0; i < 8; i++){
        if(line_detect[i] == 1U){
            line_data += (float)i;
            line_geshu += 1.0f;
        }
    }

    if(line_geshu > 0.0f){
        line_value = line_data / line_geshu;
        last_line_value = line_value;
        line_lost_timing = 0U;
        line_lost_start_ms = 0U;
			  line_flag = 1;
    }else {
        line_value = last_line_value;//防止小车短暂脱离黑条时循迹值突变

        now_ms = HAL_GetTick();
        if(line_lost_timing == 0U){
            line_lost_timing = 1U;
            line_lost_start_ms = now_ms;
            line_flag = 1;
        }else if((uint32_t)(now_ms - line_lost_start_ms) >= LINE_LOST_CONFIRM_MS){
            line_flag = 0;
        }else {
            line_flag = 1;
        }
    }
}

/**
 * @brief 执行一次循迹控制进程。
 * @property 根据 line_value 计算左右轮速度差，并通过 App_Speed_Set 输出目标速度。
 */
void App_Line_Follow_Proc(void){
    Line_Follow_UpdateData();

	if(line_flag == 1){
		float change_speed = 0.0f;
    float left_speed = 0.0f;
    float right_speed = 0.0f;

    change_speed = PID_Compute(&line_follow_pid, line_value);

    left_speed = speed_base + change_speed;
    right_speed = speed_base - change_speed;

    App_Speed_Set(left_speed, right_speed);
	}else if(line_flag == 0){
			Motor_Stop();
	}

}
