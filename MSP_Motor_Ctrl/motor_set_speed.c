#include "motor_set_speed.h"




void Motor_Set_ClosedLoop(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x06;        // ¹¦ÄÜÂë£ºĞ´µ¥¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x08;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

  
    // ¼Ä´æÆ÷ 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}



void Motor_Set_Speeds(int16_t v0, int16_t v1, int16_t v2, int16_t v3)
{
    uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x10;        // ¹¦ÄÜÂë£ºĞ´¶à¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

    frame[idx++] = 0x00;        // ¼Ä´æÆ÷ÊıÁ¿¸ß×Ö½Ú
    frame[idx++] = 0x04;        // ¼Ä´æÆ÷ÊıÁ¿µÍ×Ö½Ú£¨4 ¸ö£©
	
    frame[idx++] = 0x08;        // Êı¾İ×Ö½ÚÊı = 4 ¡Á 2 = 8


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

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}


/*
 * å‡½æ•°åï¼šMotor_Clear_Encoders
 * åŠŸ  èƒ½ï¼šåŒæ—¶æ¸…é›¶ 4 ä¸ªç”µæœºçš„ç´¯è®¡ç¼–ç å™¨æ•°æ®
 * å  è®®ï¼šå‘å¯„å­˜å™¨ 0x0004 å¼€å§‹çš„ 4 ä¸ªå¯„å­˜å™¨å†™å…¥ 0
 */
void Motor_Clear_Encoders(void)
{
    uint8_t idx = 0;
    uint8_t frame[20];

    frame[idx++] = 0x0A;        // ä»ç«™åœ°å€ (é©±åŠ¨å™¨ ID)
    frame[idx++] = 0x10;        // åŠŸèƒ½ç ï¼šå†™å¤šä¸ªä¿æŒå¯„å­˜å™¨

    frame[idx++] = 0x00;        // èµ·å§‹å¯„å­˜å™¨é«˜å­—èŠ‚
    frame[idx++] = 0x04;        // èµ·å§‹å¯„å­˜å™¨ä½å­—èŠ‚ (åœ°å€ 0x0004)

    frame[idx++] = 0x00;        // å¯„å­˜å™¨æ•°é‡é«˜å­—èŠ‚
    frame[idx++] = 0x04;        // å¯„å­˜å™¨æ•°é‡ä½å­—èŠ‚ï¼ˆå…± 4 ä¸ªå¯„å­˜å™¨ï¼‰
	
    frame[idx++] = 0x08;        // æ•°æ®å­—èŠ‚æ•° = 4 Ã— 2 = 8

    // æ•°æ®åŒºï¼šå…¨éƒ¨å¡«å…¥ 0x00ï¼Œä»£è¡¨æ¸…é›¶
    frame[idx++] = 0x00;        // ç”µæœº 1 ç´¯è®¡ç¼–ç å™¨é«˜å­—èŠ‚
    frame[idx++] = 0x00;        // ç”µæœº 1 ç´¯è®¡ç¼–ç å™¨ä½å­—èŠ‚

    frame[idx++] = 0x00;        // ç”µæœº 2 ç´¯è®¡ç¼–ç å™¨é«˜å­—èŠ‚
    frame[idx++] = 0x00;        // ç”µæœº 2 ç´¯è®¡ç¼–ç å™¨ä½å­—èŠ‚

    frame[idx++] = 0x00;        // ç”µæœº 3 ç´¯è®¡ç¼–ç å™¨é«˜å­—èŠ‚
    frame[idx++] = 0x00;        // ç”µæœº 3 ç´¯è®¡ç¼–ç å™¨ä½å­—èŠ‚

    frame[idx++] = 0x00;        // ç”µæœº 4 ç´¯è®¡ç¼–ç å™¨é«˜å­—èŠ‚
    frame[idx++] = 0x00;        // ç”µæœº 4 ç´¯è®¡ç¼–ç å™¨ä½å­—èŠ‚

    // è®¡ç®— CRC æ ¡éªŒ
    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;          // CRC ä½å­—èŠ‚ (CRC_L)
    frame[idx++] = (crc >> 8) & 0xFF;   // CRC é«˜å­—èŠ‚ (CRC_H)

    // é€šè¿‡ UART å‘é€
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}


		void Motor_Set_Enc1_A(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x06;        // ¹¦ÄÜÂë£ºĞ´µ¥¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x09;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

  
    // ¼Ä´æÆ÷ 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}


void Motor_Set_Enc1_B(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x06;        // ¹¦ÄÜÂë£ºĞ´µ¥¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x0A;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

  
    // ¼Ä´æÆ÷ 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}

void Motor_Set_Enc1_C(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x06;        // ¹¦ÄÜÂë£ºĞ´µ¥¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x0B;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

  
    // ¼Ä´æÆ÷ 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}

void Motor_Set_Enc1_D(void)
{

		uint8_t idx = 0;
		uint8_t frame[20];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x06;        // ¹¦ÄÜÂë£ºĞ´µ¥¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;        // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
    frame[idx++] = 0x0C;        // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú

  
    // ¼Ä´æÆ÷ 1
    frame[idx++] = 0x00;
    frame[idx++] = 0x01;

    uint16_t crc = CRC16(frame, idx);
    frame[idx++] = crc & 0xFF;
    frame[idx++] = (crc >> 8) & 0xFF;

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}












void Motor_Set_KP_KI_KD(PID_t *Motor1, PID_t *Motor2, PID_t *Motor3, PID_t *Motor4)
{
		uint16_t Kp_Temp;
		uint16_t Ki_Temp;
		uint16_t Kd_Temp;

    uint8_t idx = 0;
		uint8_t frame[34];

    frame[idx++] = 0x0A;        // ´ÓÕ¾µØÖ·
    frame[idx++] = 0x10;        // ¹¦ÄÜÂë£ºĞ´¶à¸ö±£³Ö¼Ä´æÆ÷

    frame[idx++] = 0x00;  // ÆğÊ¼¼Ä´æÆ÷¸ß×Ö½Ú
		frame[idx++] = 0x15;  // ÆğÊ¼¼Ä´æÆ÷µÍ×Ö½Ú (21)
	
    frame[idx++] = 0x00;        // ¼Ä´æÆ÷ÊıÁ¿¸ß×Ö½Ú
    frame[idx++] = 0x0C;        // ¼Ä´æÆ÷ÊıÁ¿µÍ×Ö½Ú£¨12 ¸ö£©
	
    frame[idx++] = 0x18;   // Êı¾İ×Ö½ÚÊı = 12 ¡Á 2 = 24


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

    // ·¢ËÍ
    for (uint8_t i = 0; i < idx; i++)
    {
        while (DL_UART_isBusy(MSPMotor_INST));
        DL_UART_Main_transmitData(MSPMotor_INST, frame[i]);
    }
}