#include "ti_msp_dl_config.h"
#include "stdio.h"
#include "clock.h"
#include "task.h"
#include "imu.h"
#include "app_vl5310x.h"
#include "app_linedetect.h"
#include "app_PWM.h"
#include "app_basic.h"
#include "app_extend.h"
#include "keil/app_button.h"
#include "keil/app_bat.h"

#include "app_motor.h"
#include "app_encoder.h"
#include "motor.h"
#include "app_task.h"
#include "app_speed.h"
#include "app_task4.h"
#include "vision.h"
volatile unsigned char imu_data = 0;

extern volatile uint16_t VL5310X_Distance_mm[VL5310X_COUNT];//测距模块
uint8_t task3_set_exit_flag = 0;//用来任务3出口的标志位
uint8_t task4_set_exit_flag = 0;//用来任务四出口的标志位
uint8_t task3_started = 0;//任务3是否已收到相机 0x77 并开始跑
uint8_t task4_started = 0;//任务4是否已收到相机 0x77 并开始跑
volatile uint8_t set_the_exit_of_task4_flag = 2;//用来设置任务四出口的标志位，1->(3,0)  2->(3,1) ,3->(3,2),4->(3,3)
int main(void)
{
    SYSCFG_DL_init();

    SysTick_Init();
    App_Encoder_Init();
    Vision_Init();
    IMU_Init();
    App_VL5310X_Init();
    App_PWM_Init();
    Motor_Init();
    App_Button_Init();
		
		
		delay_ms(100);

		//设置初始pid
		App_Motor_Init();		
	
		App_Speed_Init();
		
		App_Speed_Set(0,0);
    App_Speed_Reset();

		uint32_t time = HAL_GetTick();
		int i = 0;
    while (1)
    {	
			
			///////////////////////////////////
			//////////////////////////////////
			//翻转小灯电平，看看程序卡住了没
				PERIODIC_START(LED,500)
            DL_GPIO_togglePins(GPIOB, DL_GPIO_PIN_22);
        PERIODIC_END
			///////////////////////////////////
			////检测灰度，//黑线为1，白线为0
			//////////////////////////////////
			 PERIODIC_START(LINE_DETECT,30)
            Line_detect();
        PERIODIC_END
			///////////////////////////////////
			////检测按键
			//////////////////////////////////
			 PERIODIC_START(BUTTON,100)
            App_Button_Proc();
        PERIODIC_END
			///////////////////////////////////
			//////////////////////////////////
			//打印灰度值，用来测试
//			  PERIODIC_START(LINE,200)
//            printf("%d,%d,%d,%d,%d,%d,%d,%d\n",line_detect[0],line_detect[1],line_detect[2],line_detect[3],line_detect[4],line_detect[5],line_detect[6],line_detect[7]);
//        PERIODIC_END
			///////////////////////////////////
			//////////////////////////////////
			
				///////////////////////////////////
			//////////////////////////////////
			//打印角度值，用来测试
			  PERIODIC_START(LINE,50)
				float yaw = getYaw();
				float pitch = getPitch();
				float roll = getRoll();
            printf("%f,%f,%f\n",yaw,pitch,roll);
        PERIODIC_END
		}

}
