#include "imu.h"
#include "clock.h"
#include "ti_msp_dl_config.h"

struct SAngle stcAngle;
struct SGyro stcGyro;

void IMU_Init(void)
{
    // 【修改点3】把 IMU 串口接收的优先级提拔到最高！(0 级)
    NVIC_SetPriority(IMU_INST_INT_IRQN, 0);

    // 【修改点4】清理上电期间或 delay_ms 期间积攒的垃圾数据和溢出报错
    DL_UART_clearInterruptStatus(IMU_INST, 0xFFFFFFFF); // 清除所有潜在的错误标志
    while (DL_UART_isRXFIFOEmpty(IMU_INST) == false) {
        DL_UART_Main_receiveData(IMU_INST); // 疯狂读取，直到把底层 FIFO 里的垃圾排空
    }

    NVIC_ClearPendingIRQ(IMU_INST_INT_IRQN);
    NVIC_EnableIRQ(IMU_INST_INT_IRQN);
    
    // 初始化时校准偏置（此时可以允许 21s 阻塞）
    //performCaliBias();
    
    // Z轴角度归零
    sendCaliYawCommand(); // 注意：这个函数里的 delay_ms 现在就算引发了硬件溢出，也会被下面的中断修复！
}

void IMU_send_char(char ch)
{
    while( DL_UART_isBusy(IMU_INST) == true );
    DL_UART_Main_transmitData(IMU_INST, ch);
}

void IMU_send_bytes(uint8_t* data, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        IMU_send_char(data[i]);
    }
}
//串口发送字符串
void IMU_send_string(char* str)
{
    //当前字符串地址不在结尾 并且 字符串首地址不为空
    while(*str!=0&&str!=0)
    {
        //发送字符串首地址中的字符，并且在发送完成之后首地址自增
       IMU_send_char(*str++);
    }
}


/******************************************************************************
 * 数据解析函数：接收0x5A开头的5字节数据帧
******************************************************************************/
void CopeSerial2Data(unsigned char ucData)
{
    static unsigned char ucRxBuffer[11];
    static unsigned char ucRxCnt = 0;

    ucRxBuffer[ucRxCnt++] = ucData;

    if (ucRxBuffer[0] != 0x5A)
    {
        ucRxCnt = 0;
        return;
    }

    if (ucRxCnt < 5) return;

    unsigned char sum = 0;
    if (ucRxBuffer[1] == 0xAA)
    {
        // 角速度帧校验和：0x5A + 0xAA + AzL + AzH 
        sum = ucRxBuffer[0] + ucRxBuffer[1] +
              ucRxBuffer[2] + ucRxBuffer[3] ;

        if (sum != ucRxBuffer[4])
        {
            ucRxCnt = 0;
            return;
        }

        short wz    = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);

        stcGyro.wz    = (float)wz    / 32768.0f * 2000.0f;
    }
    else if (ucRxBuffer[1] == 0xBB)
    {
        // 角度帧校验和：0x5A + 0xBB + YawH + YawL 
        sum = ucRxBuffer[0] + ucRxBuffer[1] +
              ucRxBuffer[2] + ucRxBuffer[3];

        if (sum != ucRxBuffer[4])
        {
            ucRxCnt = 0;
            return;
        }

        short rawYaw = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);
        stcAngle.Yaw = (float)rawYaw / 32768.0f * 180.0f;
    }
    ucRxCnt = 0;
}
//void CopeSerial2Data(unsigned char ucData)
//{
//	static unsigned char ucRxBuffer[5];
//	static uint8_t state = 0;
//	
//	switch(state)
//	{
//	  case 0:
//		if(ucData == 0x5A){
//			ucRxBuffer[0]=ucData;
//			state =1;
//		}
//		break;
//		case 1:
//			if(ucData==0xAA||ucData ==0xBB)
//			{
//				ucRxBuffer[1]=ucData;
//				state = 2;
//				
//			}
//			else
//      {
//				state = 0;
//      }
//	  break;
//		case 3:
//			ucRxBuffer[3]=ucData;
//		  state =4;
//		  break;
//	   case 4:
//			 ucRxBuffer[4] = ucData;
//		 
//		 unsigned char sum = ucRxBuffer[0]+ucRxBuffer[1]+ucRxBuffer[2]+ucRxBuffer[3];
//		 if(sum == ucRxBuffer[4])
//		 {
//			 if(ucRxBuffer[1]==0xAA){
//				 short wz =(short)((ucRxBuffer[3]<<8)|ucRxBuffer[2]);
//				 stcGyro.wz = (float)wz/32768.0f*2000.0f;
//			 }
//			 else if(ucRxBuffer[1]==0xBB)
//			 {
//				 short rawYaw =(short)((ucRxBuffer[3]<<8)|ucRxBuffer[2]);
//				 stcAngle.Yaw = (float)rawYaw/32768.0f*180.0f;
//			 }
//		 }
//		 state = 0;
//		 break;
//		 
//		 default:
//		state = 0;
//		 break;
//	}
//	
//}



// 获取yaw轴角度
float getYaw(void)
{
    return stcAngle.Yaw;
}
// 获取z轴角加速度
float getGyroZ(void)
{
    return stcGyro.wz;
}

/********************* IMU 写格式  ************************/
uint8_t Key[5]       = {0x55, 0xAA, 0x13, 0x8E, 0x5F}; // 解锁指令（解锁，往该寄存器写0x8E5F（其他值无效））
uint8_t Yaw_Zero[5]  = {0x55, 0xAA, 0x15, 0x00, 0x00}; // Z轴归零指令
uint8_t Save[5]      = {0x55, 0xAA, 0x00, 0x00, 0x00}; // 保存指令
uint8_t BIAS_CAL[5]  = {0x55, 0xAA, 0x0A, 0x01, 0x00}; // 获取零偏指令


/******************************************************************************
 * 发送 Z轴角度归零命令
 ******************************************************************************/
void sendCaliYawCommand(void)
{
    IMU_send_bytes(Key, 5);
    delay_ms(100);
    IMU_send_bytes(Yaw_Zero, 5);
    delay_ms(100);
    IMU_send_bytes(Save, 5);
}

/******************************************************************************
 * 发送自动获取零偏校准指令(校准过程中请勿移动)
 ******************************************************************************/
void performCaliBias(void)
{
    IMU_send_bytes(Key, 5);
    delay_ms(100);
    IMU_send_bytes(BIAS_CAL, 5);
    delay_ms(21000); // 传感器需要约 20 秒稳定时间
    IMU_send_bytes(Save, 5);
}



// 延时函数封装映射
void delay_ms(int __ms)  { mspm0_delay_ms(__ms); }
void delay_us(int __us)  { delay_cycles((CPUCLK_FREQ / 1000000) * __us); }
