#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "motor_read_enc.h"
#include "motor_set_speed.h"
#include "clock.h"   // 包含 mspm0_delay_ms 和 SysTick_Init
#include "task.h"    // 包含基于 SysTick 的任务调度宏
#include "imu.h"
#include "app_vl5310x.h"
#include "app_linedetect.h"
#include "app_PWM.h"
#include "app_basic.h"
#include "app_extend.h"
#include "keil/app_motor.h"
#include "keil/app_button.h"
#include "keil/app_bat.h"
#include "sast_lib_st7735s.h"

volatile unsigned char uart_data = 0;

int16_t MA_Speed, MB_Speed, MC_Speed, MD_Speed;


PID_t M1_PID;
PID_t M2_PID;
PID_t M3_PID;
PID_t M4_PID;

int main(void)
{
    SYSCFG_DL_init();	

    /* 1. 初始化 SysTick，启动底层 1ms 滴答定时器 */
    SysTick_Init();

    // 初始化陀螺仪（打开中断并校准），要静止30s左右
    IMU_Init();

    // 初始化三路 VL53L0X，后续在主循环里轮询读取
    App_VL5310X_Init();

    // 初始化四路 180 度舵机 PWM，默认全部置中到 90 度
    App_PWM_Init();
	 //灰度传感器的初始化就是引脚初始化，sysconfig里已经写好，ti会自动配置
	 //App_Linedetect_Init();  写个假的，看起来顺眼
    App_Motor_Init();
    App_Basic_Init();
    App_Extend_Init();
    App_Button_Init();
    App_Bat_Init();

    // 初始化电机驱动板的串口（打开中断）
    Motor_Init();

    /* 2. 初始化阶段的阻塞延时：使用基于 SysTick 的 mspm0_delay_ms */
    Motor_Set_ClosedLoop();
    mspm0_delay_ms(50);     // 依靠 tick_ms 的阻塞延时

    M1_PID.kp = 40.0;
    M2_PID.kp = 40.0;
    M3_PID.kp = 40.0;
    M4_PID.kp = 40.0;
    M1_PID.ki = 4.9;
    M2_PID.ki = 4.9;
    M3_PID.ki = 4.9;
    M4_PID.ki = 4.9;
		//任务标志位
		extern uint8_t task1_flag;
		extern uint8_t task2_flag;
		extern uint8_t task3_flag;
		extern uint8_t task4_flag;
		//外部变量
		extern volatile uint16_t VL5310X_Distance_mm[VL5310X_COUNT];
		extern volatile uint8_t line_raw[LINE_SENSOR_NUM];
		
		extern const unsigned char g_yawangle_1616_sast[];
		extern const unsigned char g_voltage_1616_sast[];
		st7735s_init();
		st7735s_fill_rect(0, 0, ST7735S_W -1, ST7735S_H -1, ST7735S_BLACK );
		st7735s_draw_chinese_string(0, 10, g_yawangle_1616_sast , 3, ST7735S_WHITE,ST7735S_BLACK,16,16,ST7735S_NON_OVERLAY_MODE);
		
		st7735s_draw_chinese_string(0, 40, g_voltage_1616_sast , 5, ST7735S_WHITE,ST7735S_BLACK,16,16,ST7735S_NON_OVERLAY_MODE);
		
	
		
		DL_GPIO_setPins(LED_PORT,LED_LED_0_PIN);

    while (1) {
//			 PERIODIC_START(task_led, 200)
//            DL_GPIO_togglePins(LED_PORT,LED_LED_0_PIN);
//        PERIODIC_END
//			
//			按键已测试过，可以正常用
//			if(task1_flag == 1)
//			{
//				DL_GPIO_togglePins(LED_PORT,LED_LED_0_PIN);
//				task1_flag = 0;
//			}
//			if(task2_flag == 1)
//			{
//				DL_GPIO_togglePins(LED_PORT,LED_LED_0_PIN);
//				task2_flag = 0;
//			}
//			if(task3_flag == 1)
//			{
//				DL_GPIO_togglePins(LED_PORT,LED_LED_0_PIN);
//				task3_flag = 0;
//			}
//			if(task4_flag == 1)
//			{
//				DL_GPIO_togglePins(LED_PORT,LED_LED_0_PIN);
//				task4_flag = 0;
//			}

			  App_Button_Proc();//一直扫描按键
			  App_Bat_Proc();
				
				
				//显示角度
        PERIODIC_START(task_lcd_yaw, 100)
            float yaw_lcd = getYaw();
            int16_t yaw10 = (int16_t)(yaw_lcd * 10.0f);

            st7735s_fill_rect(48, 10, ST7735S_W - 1, 25, ST7735S_BLACK);
            if (yaw10 < 0)
            {
                yaw10 = -yaw10;
                st7735s_printf(48, 10, ST7735S_WHITE, ST7735S_BLACK,
                               ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE,
                               "-%d.%d", yaw10 / 10, yaw10 % 10);
            }
            else
            {
                st7735s_printf(48, 10, ST7735S_WHITE, ST7735S_BLACK,
                               ST7735S_SIZE_1608, ST7735S_NON_OVERLAY_MODE,
                               "%d.%d", yaw10 / 10, yaw10 % 10);
            }
        PERIODIC_END
						
						
						
//        /* 3. 任务一：电机控制非阻塞任务，基于 SysTick (tick_ms)
//         * 每 10ms 调度一次，PID和速度交替发送 */
//        PERIODIC_START(task_motor, 10)
//            static uint8_t motor_step = 0;

//            App_Motor_Position_Proc();
//            App_Motor_Angle_Proc();

//            if (motor_step == 0) {
//                Motor_Set_KP_KI_KD(&M1_PID, &M2_PID, &M3_PID, &M4_PID);
//                motor_step = 1;
//            } else {
//                Motor_Set_Speeds(MA_Speed, MB_Speed, MC_Speed, MD_Speed);
//                motor_step = 0;
//            }
//        PERIODIC_END

        /* 灰度黑线检测任务：只更新黑线/边界状态，不做循迹控制 */
        PERIODIC_START(task_line_detect, 20)
            App_LineDetect_Proc();
        PERIODIC_END

//        /* VL53L0X 轮询任务：更新 VL5310X_Distance_mm[VL5310X_FRONT/LEFT/RIGHT] */
        PERIODIC_START(task_vl53l0x, 50)
            App_VL5310X_Proc();
        PERIODIC_END

//        PERIODIC_START(task_app, 20)
//            App_Basic_Proc();
//            App_Extend_Proc();
//        PERIODIC_END

        /* 串口打印非阻塞，基于 SysTick (tick_ms) */
        PERIODIC_START(task_print, 100)
						//电机编码器
//            printf("%d,%d,%d,%d,%d,%d,%d,%d\n",
//                   modbus_date[0], modbus_date[1], modbus_date[2], modbus_date[3],
//                   modbus_date[4], modbus_date[5], modbus_date[6], modbus_date[7]);

//						//陀螺仪
									 float yaw_angle = getYaw();
									 float a = 1;
						printf("%lf,%lf\n",yaw_angle,a);
					
					float volt_adc = App_Bat_Get();
				//	printf("%lf\n",volt_adc);
//						
						////灰度传感器
//            printf("%d,%d,%d,%d,%d,%d,%d,%d\n",
//                line_raw[0], line_raw[1], line_raw[2], line_raw[3],
//                line_raw[4], line_raw[5], line_raw[6], line_raw[7]);
//						
						//激光测距
						//printf("%d,%d,%d\n",VL5310X_Distance_mm[0],VL5310X_Distance_mm[1],VL5310X_Distance_mm[2]);
						
									 
									 
        PERIODIC_END

    }
}
