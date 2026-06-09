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
volatile unsigned char imu_data = 0;

extern volatile uint16_t VL5310X_Distance_mm[VL5310X_COUNT];//测距模块

float angle_SP = 0;

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
					App_Speed_Set(0.0f, 0.0f);
					while(1){
					}
				}
			
				App_Update_Data();
				App_Button_Proc();//按键检测

				//灰度检测
        PERIODIC_START(task_line_detect,30)
//            App_LineDetect_Proc();
        PERIODIC_END
						
				//激光测距
        PERIODIC_START(task_vl53l0x,50)
            App_VL5310X_Proc();
        PERIODIC_END

			  if(task1_flag == 1){
					  Task1();
             				
				}else if(task2_flag == 1){
					  Task2();
				}else{
				    PERIODIC_START(task_motor_idle,30)
					 // App_Set_Speed(0.0f, 0.0f);
				    PERIODIC_END
				}

//				  if(task3_flag == 1){
//						task3_flag = 0;
//					  Task3();					
//				}
//					 if(task4_flag == 1){
//						task4_flag = 0;
//					  Task4();					
//				}

			
        PERIODIC_START(task_display, 200)
							 App_Motor_Data_Update();
				 // printf("Yaw:%.2f\n",stcAngle.Yaw);
			  printf("%d,%d,%d\n", VL5310X_Distance_mm[0],VL5310X_Distance_mm[1],VL5310X_Distance_mm[2]);
				//printf("%.2f,%.2f\n",speed_l,speed_r);
				//printf("%.2f,%.2f,%.2f,%.2f\n",
      // stcAngle.Yaw,angle_yaw_ref,position_mid,position_mid_ref);
//				printf("%.2f,%.2f\n",
//       speed_l_measure,speed_r_measure);
        PERIODIC_END
//				

    }
}
