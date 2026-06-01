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
#include "sast_lib_st7735s.h"

#include "motor_read_enc.h"
#include "motor_set_speed.h"

#include "app_motor.h"
#include "app_task.h"
volatile unsigned char uart_data = 0;
volatile unsigned char imu_data = 0;
PID_t M1_PID;
PID_t M2_PID;
PID_t M3_PID;
PID_t M4_PID;

extern volatile int16_t	modbus_date[8];//电机反馈					
extern volatile uint8_t modbus_rx_frame_done; 

uint8_t speed_tx_flag = 0;


//设置电机初始pid
static void Motor_ConfigSpeedPid(void)
{
    M1_PID.kp = 40.0f;
    M2_PID.kp = 40.0f;
    M3_PID.kp = 0.0f;
    M4_PID.kp = 0.0f;

    M1_PID.ki = 4.9f;
    M2_PID.ki = 4.9f;
    M3_PID.ki = 0.0f;
    M4_PID.ki = 0.0f;

    M1_PID.kd = 0.0f;
    M2_PID.kd = 0.0f;
    M3_PID.kd = 0.0f;
    M4_PID.kd = 0.0f;
	  Motor_Set_KP_KI_KD(&M1_PID,&M2_PID,&M3_PID,&M4_PID);
}

int main(void)
{
    SYSCFG_DL_init();

    SysTick_Init();
	
	  //开启电机串口中断
	  NVIC_SetPriority(MSPMotor_INST_INT_IRQN, 1);
	  NVIC_ClearPendingIRQ(MSPMotor_INST_INT_IRQN);
		NVIC_EnableIRQ(MSPMotor_INST_INT_IRQN);
	  // 【建议增加】清空开机时电机可能发来的垃圾数据
    DL_UART_clearInterruptStatus(MSPMotor_INST, 0xFFFFFFFF);
    while (DL_UART_isRXFIFOEmpty(MSPMotor_INST) == false) {
        DL_UART_Main_receiveData(MSPMotor_INST);
    }
    
    NVIC_ClearPendingIRQ(MSPMotor_INST_INT_IRQN);
    NVIC_EnableIRQ(MSPMotor_INST_INT_IRQN);
		
		Motor_Set_ClosedLoop(); // 设置进入速度闭环模式
    delay_ms(50);           // 现在怎么延时都不会死机了
    Motor_Clear_Encoders();
		delay_ms(50);
		
    IMU_Init();
    App_VL5310X_Init();
    App_PWM_Init();
    App_Button_Init();

//    extern const unsigned char g_yawangle_1616_sast[];

//    st7735s_init();
//    st7735s_fill_rect(0, 0, ST7735S_W - 1, ST7735S_H - 1, ST7735S_BLACK);
//    st7735s_draw_chinese_string(0, 10, g_yawangle_1616_sast, 3,
//                                ST7735S_WHITE, ST7735S_BLACK,
//                                16, 16, ST7735S_NON_OVERLAY_MODE);

    DL_GPIO_togglePins(LED_LED_1_PORT, LED_LED_1_PIN);

		delay_ms(100);
		//Motor_Set_Speeds(0,0,0,0);
		//Motor_ConfigSpeedPid();
		//设置初始pid
		//App_Motor_Init();
		int i = 0;
    while (1)
    {
//		
//			if(i == 0){
//			
//				i = 1;
//				Turn_Right_90();
//				App_Set_Speed(0,0);
//			
//			}
			
				App_Update_Data();
				//App_Button_Proc();//按键检测

        PERIODIC_START(task_led, 500) //小灯闪烁
            DL_GPIO_togglePins(LED_LED_1_PORT, LED_LED_1_PIN);
        PERIODIC_END

//				//灰度检测
//        PERIODIC_START(task_line_detect, 20)
//            App_LineDetect_Proc();
//        PERIODIC_END
						
//				//激光测距
//        PERIODIC_START(task_vl53l0x, 50)
//            App_VL5310X_Proc();
//        PERIODIC_END

//			  if(task1_flag == 1){
//					  Task1();					
//				}else if(task2_flag == 1){
//					  Task2();
//				}else{
//					  App_Set_Speed(0.0f, 0.0f);
//				}
//				  if(task3_flag == 1){
//						task3_flag = 0;
//					  Task3();					
//				}
//					 if(task4_flag == 1){
//						task4_flag = 0;
//					  Task4();					
//				}

			
        PERIODIC_START(task_imu, 50)
				 // printf("Yaw:%.2f\n",stcAngle.Yaw);
			  printf(":%d,%d,%d,%d,%.2f\n",modbus_date[0],modbus_date[1],modbus_date[4],modbus_date[5],stcAngle.Yaw);
        PERIODIC_END
				

    }
}
