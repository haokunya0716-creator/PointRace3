#include "motor_set_speed.h"

static void Motor_UART3_SendFrame(uint8_t *frame, uint8_t len)
{
    uint8_t i = 0;

    for (i = 0; i < len; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }

    while (DL_UART_isBusy(MSPMotor_INST));
}





void Motor_Set_ClosedLoop(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x06;        // ¹¦Ĝ«£ºдµ¥¸ö±£³ּĴ憷

    frame[idx++] = 0x00;        // Register address high byte.
    frame[idx++] = 0x08;        // Register address low byte.
  
    // ¼Ĵ憷 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}



void Motor_Set_Speeds(int16_t v0, int16_t v1, int16_t v2, int16_t v3)
{
    uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x10;        // ¹¦Ĝ«£ºд¶ึ±£³ּĴ憷

    frame[idx++] = 0x00;        // Start register high byte.
    frame[idx++] = 0x00;        // Start register low byte.
    frame[idx++] = 0x00;        // Register count high byte.
    frame[idx++] = 0x04;        // Register count low byte: 4 registers.
	
    frame[idx++] = 0x08;        // ʽ¾ݗֽڊý = 4 ¡Á 2 = 8


    frame[idx++] = (v0 >> 8) & 0xFF;
    frame[idx++] = (v0 >> 0) & 0xFF;


    frame[idx++] = (v1 >> 8) & 0xFF;
    frame[idx++] = (v1 >> 0) & 0xFF;

    frame[idx++] = (v2 >> 8) & 0xFF;
    frame[idx++] = (v2 >> 0) & 0xFF;

    frame[idx++] = (v3 >> 8) & 0xFF;
    frame[idx++] = (v3 >> 0) & 0xFF;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}


/*
 * 函数名：Motor_Clear_Encoders
 * 功  能：同时清零 4 个电机的累计编码器数据
 * 协  议：向寄存器 0x0004 开始的 4 个寄存器写入 0
 */
void Motor_Clear_Encoders(void)
{
    uint8_t idx = 0;
    uint8_t frame[20];

    frame[idx++] = 0x0A;        // 从站地址 (驱动器 ID)
    frame[idx++] = 0x10;        // 功能码：写多个保持寄存器

    frame[idx++] = 0x00;        // 起始寄存器高字节
    frame[idx++] = 0x04;        // 起始寄存器低字节 (地址 0x0004)

    frame[idx++] = 0x00;        // 寄存器数量高字节
    frame[idx++] = 0x04;        // 寄存器数量低字节（共 4 个寄存器）
	
    frame[idx++] = 0x08;        // 数据字节数 = 4 × 2 = 8

    // 数据区：全部填入 0x00，代表清零
    frame[idx++] = 0x00;        // 电机 1 累计编码器高字节
    frame[idx++] = 0x00;        // 电机 1 累计编码器低字节

    frame[idx++] = 0x00;        // 电机 2 累计编码器高字节
    frame[idx++] = 0x00;        // 电机 2 累计编码器低字节

    frame[idx++] = 0x00;        // 电机 3 累计编码器高字节
    frame[idx++] = 0x00;        // 电机 3 累计编码器低字节

    frame[idx++] = 0x00;        // 电机 4 累计编码器高字节
    frame[idx++] = 0x00;        // 电机 4 累计编码器低字节

    // 计算 CRC 校验
    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;          // CRC 低字节 (CRC_L)
    frame[idx++] = (crc >> 8) & 0xFF;   // CRC 高字节 (CRC_H)

    // 通过 UART 发送
    Motor_UART3_SendFrame(frame, idx);
}


		void Motor_Set_Enc1_A(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x06;        // ¹¦Ĝ«£ºдµ¥¸ö±£³ּĴ憷

    frame[idx++] = 0x00;        // Register address high byte.
    frame[idx++] = 0x09;        // Register address low byte.
  
    // ¼Ĵ憷 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}


void Motor_Set_Enc1_B(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x06;        // ¹¦Ĝ«£ºдµ¥¸ö±£³ּĴ憷

    frame[idx++] = 0x00;        // Register address high byte.
    frame[idx++] = 0x0A;        // Register address low byte.
  
    // ¼Ĵ憷 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}

void Motor_Set_Enc1_C(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x06;        // ¹¦Ĝ«£ºдµ¥¸ö±£³ּĴ憷

    frame[idx++] = 0x00;        // Register address high byte.
    frame[idx++] = 0x0B;        // Register address low byte.
  
    // ¼Ĵ憷 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}

void Motor_Set_Enc1_D(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x06;        // ¹¦Ĝ«£ºдµ¥¸ö±£³ּĴ憷

    frame[idx++] = 0x00;        // Register address high byte.
    frame[idx++] = 0x0C;        // Register address low byte.
  
    // ¼Ĵ憷 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}












void Motor_Set_KP_KI_KD(PID_t *Motor1, PID_t *Motor2, PID_t *Motor3, PID_t *Motor4)
{
		uint16_t Kp_Temp;
		uint16_t Ki_Temp;
		uint16_t Kd_Temp;

    uint8_t idx = 0;
		uint8_t frame[34];

    frame[idx++] = 0x0A;        // ´ӕ¾µؖ·
    frame[idx++] = 0x10;        // ¹¦Ĝ«£ºд¶ึ±£³ּĴ憷

    frame[idx++] = 0x00;        // Start register high byte.
    frame[idx++] = 0x15;        // Start register low byte: 0x0015.
	
    frame[idx++] = 0x00;        // Register count high byte.
    frame[idx++] = 0x0C;        // Register count low byte: 12 registers.
	
    frame[idx++] = 0x18;   // ʽ¾ݗֽڊý = 12 ¡Á 2 = 24


		Kp_Temp = (uint16_t)(Motor1->kp * 1000);
		Ki_Temp = (uint16_t)(Motor1->ki * 1000);
		Kd_Temp = (uint16_t)(Motor1->kd * 1000);

    frame[idx++] = (Kp_Temp >> 8) & 0xFF;
    frame[idx++] = (Kp_Temp >> 0) & 0xFF;


    frame[idx++] = (Ki_Temp >> 8) & 0xFF;
    frame[idx++] = (Ki_Temp >> 0) & 0xFF;

    frame[idx++] = (Kd_Temp >> 8) & 0xFF;
    frame[idx++] = (Kd_Temp >> 0) & 0xFF;


		Kp_Temp = (uint16_t)(Motor2->kp * 1000);
		Ki_Temp = (uint16_t)(Motor2->ki * 1000);
		Kd_Temp = (uint16_t)(Motor2->kd * 1000);

    frame[idx++] = (Kp_Temp >> 8) & 0xFF;
    frame[idx++] = (Kp_Temp >> 0) & 0xFF;


    frame[idx++] = (Ki_Temp >> 8) & 0xFF;
    frame[idx++] = (Ki_Temp >> 0) & 0xFF;

    frame[idx++] = (Kd_Temp >> 8) & 0xFF;
    frame[idx++] = (Kd_Temp >> 0) & 0xFF;



		Kp_Temp = (uint16_t)(Motor3->kp * 1000);
		Ki_Temp = (uint16_t)(Motor3->ki * 1000);
		Kd_Temp = (uint16_t)(Motor3->kd * 1000);

    frame[idx++] = (Kp_Temp >> 8) & 0xFF;
    frame[idx++] = (Kp_Temp >> 0) & 0xFF;


    frame[idx++] = (Ki_Temp >> 8) & 0xFF;
    frame[idx++] = (Ki_Temp >> 0) & 0xFF;

    frame[idx++] = (Kd_Temp >> 8) & 0xFF;
    frame[idx++] = (Kd_Temp >> 0) & 0xFF;


		Kp_Temp = (uint16_t)(Motor4->kp * 1000);
		Ki_Temp = (uint16_t)(Motor4->ki * 1000);
		Kd_Temp = (uint16_t)(Motor4->kd * 1000);

    frame[idx++] = (Kp_Temp >> 8) & 0xFF;
    frame[idx++] = (Kp_Temp >> 0) & 0xFF;


    frame[idx++] = (Ki_Temp >> 8) & 0xFF;
    frame[idx++] = (Ki_Temp >> 0) & 0xFF;

    frame[idx++] = (Kd_Temp >> 8) & 0xFF;
    frame[idx++] = (Kd_Temp >> 0) & 0xFF;












   
    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ˍ
    Motor_UART3_SendFrame(frame, idx);
}
