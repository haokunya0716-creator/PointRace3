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
			if(i == 0)
			{
					i++;
			}

//	  	if(HAL_GetTick() - time < 5000){
//					//App_Speed_Set(45,45);
//				//Motor_Set_R(30);
//				//angle_SP = 90.0f;
//				
//				if(i == 0){
//				i++;
//					Set_Position_SP(80);
//				}
//			}else if(HAL_GetTick() - time < 8000){
//				//Motor_Set_R(50);
//				//App_Speed_Set(15,15);
//				angle_SP = -90.0f;
//				
//				if(i == 1){
//				i++;
//					Set_Position_SP(45);
//				}
//				
//			}else if(HAL_GetTick() - time < 14000){
//				//Motor_Set_R(10);
//				//App_Speed_Set(0,0);
//					//angle_SP = 180.0f;
//				if(i == 2){
//				i++;
//				Set_Position_SP(-125.0);
//				}
//				
//			}else if(HAL_GetTick() - time > 150000 ){
//				//i = 0;
//				time = HAL_GetTick();
//			}
//			
			//App_Angle_Pro();
			//App_Speed_Pro();
			//App_Position_Pro();
				if(stop_flag == 1){
					stop_flag = 0;
					task1_flag = 0;
					task2_flag = 0;
					task3_flag = 0;
					task4_flag = 0;
					App_Speed_Set(0.0f, 0.0f);
					while(1){
					}
				}
			
				App_Update_Data();
				App_Button_Proc();//按键检测
					
				//激光测距
        PERIODIC_START(task_vl53l0x,50)
            App_VL5310X_Proc();
        PERIODIC_END

			  if(task1_flag == 1){
					  Task1();
              				
				}else if(task2_flag == 1){
					  Task2();
				}else if(task3_flag == 1){
					 if(task3_set_exit_flag == 0){
						 Vision_SendCommand(0x06);

						task3_set_exit_flag = 1;

					}

					 if(task3_started == 0 && vision.cmd == 0x07){
						 task3_started = 1;
					 }

					 if(task3_started){
						  Task4(2);//默认出口
					 }

				}else if(task4_flag == 1){
					if(task4_set_exit_flag == 0){

						Vision_SendCommand(0x08);

						task4_set_exit_flag = 1;

					}

						 if(task4_started == 0 && vision.cmd == 0x07){
							 task4_started = 1;
						 }

						 if(task4_started){
						 Task4(set_the_exit_of_task4_flag);}
					 }

				else
		{
					task3_set_exit_flag = 0;
					task3_started = 0;
					task4_set_exit_flag = 0;
					task4_started = 0;

				    PERIODIC_START(task_motor_idle,30)
					 // App_Set_Speed(0.0f, 0.0f);
				    PERIODIC_END
				}

//				  if(task3_flag == 1){
//						task3_flag = 0;
//					  Task3();					
//				}
//					 if(task4_flag == 1){
//					  Task4();					
//				}

			
//        PERIODIC_START(task_display, 200)
//							 App_Motor_Data_Update();
//				 // printf("Yaw:%.2f\n",stcAngle.Yaw);
//			  printf("%d,%d,%d\n", VL5310X_Distance_mm[0],VL5310X_Distance_mm[1],VL5310X_Distance_mm[2]);
				//printf("%.2f,%.2f\n",speed_l,speed_r);
				//printf("%.2f,%.2f,%.2f,%.2f\n",
      // stcAngle.Yaw,angle_yaw_ref,position_mid,position_mid_ref);
//				printf("%.2f,%.2f\n",
//       speed_l_measure,speed_r_measure);
//        PERIODIC_END
				// 验证小车主循环程序有没有卡死
        PERIODIC_START(manba_out,500)
           DL_GPIO_togglePins(LED_LED_0_PORT,LED_LED_0_PIN);
					 DL_GPIO_togglePins(LED_LED_1_PORT,LED_LED_1_PIN);
        PERIODIC_END

    }
}
