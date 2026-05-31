#include "motor_Read_enc.h"


volatile int16_t	modbus_date[8];		//å‰å››ä¸ªæ˜¯ç¼–ç å™¨å€¼ï¼Œåå››ä¸ªæ˜¯ç”µæœºé€Ÿåº¦å€¼ï¼ˆå•ä½æ¯ç¼–ç å™¨æ¯ç§’ï¼‰
volatile uint8_t modbus_rx_frame_done;   // ±àÂëÆ÷¶ÁÈ¡Íê³É±êÖ¾Î»



volatile int16_t modbus_date[8];       // ´æ´¢½ÓÊÕµ½µÄ¼Ä´æÆ÷Êı¾İ
volatile uint8_t modbus_rx_frame_done; // ½ÓÊÕÍê³É±êÖ¾

#define MODBUS_MAX_REG 8

void Modbus_ParseFrame(uint8_t data)
{
    /* ½ÓÊÕ×´Ì¬»ú×´Ì¬ */
    static uint8_t state = 0;
    static uint8_t modbus_reg_num;

    /* ÒÑ½ÓÊÕ¼Ä´æÆ÷ÊıÁ¿Ë÷Òı */
    static uint8_t data_idx = 0;

    /* ÁÙÊ±¼Ä´æÆ÷£¬ÓÃÓÚÆ´½Ó¸ßµÍ×Ö½Ú */
    static uint16_t reg_temp = 0;

    /* ½ÓÊÕµ½µÄ CRC µÍ / ¸ß×Ö½Ú */
    static uint8_t crc_l = 0;
    static uint8_t crc_h = 0;

    /* Ô­Ê¼½ÓÊÕÊı¾İ»º´æ£¬ÓÃÓÚ CRC Ğ£Ñé */
    static uint8_t modbus_raw_buf[32];

    /* Ô­Ê¼Êı¾İË÷Òı */
    static uint8_t raw_idx = 0;

    /* Modbus Êı¾İÇø×Ö½ÚÊı */
    static uint8_t modbus_rx_byte_cnt = 0;

    /* ÁÙÊ±¼Ä´æÆ÷»º³åÇø */
    static uint16_t modbus_reg_buf[MODBUS_MAX_REG];

    /* ±£´æÔ­Ê¼Êı¾İÓÃÓÚ CRC ¼ÆËã */
    if (raw_idx < sizeof(modbus_raw_buf))
        modbus_raw_buf[raw_idx++] = data;
    else
        raw_idx = 0;

    switch (state)
    {
        case 0: // ½ÓÊÕ´ÓÕ¾µØÖ·
            if (data == 0x0A)
                state = 1;
            else
                raw_idx = 0;
            break;

        case 1: // ½ÓÊÕ¹¦ÄÜÂë
            if (data == 0x03)
                state = 2;
            else
            {
                state = 0;
                raw_idx = 0;
            }
            break;

        case 2: // ½ÓÊÕÊı¾İ×Ö½ÚÊı
            modbus_rx_byte_cnt = data;

            if ((modbus_rx_byte_cnt & 0x01) ||
                (modbus_rx_byte_cnt / 2 > MODBUS_MAX_REG))
            {
                state = 0;
                raw_idx = 0;
                break;
            }

            modbus_reg_num = modbus_rx_byte_cnt / 2;
            data_idx = 0;
            state = 3;
            break;

        case 3: // ½ÓÊÕ¼Ä´æÆ÷¸ß×Ö½Ú
            reg_temp = ((uint16_t)data << 8);
            state = 4;
            break;

        case 4: // ½ÓÊÕ¼Ä´æÆ÷µÍ×Ö½Ú
            reg_temp |= data;

            if (data_idx < MODBUS_MAX_REG)
                modbus_reg_buf[data_idx++] = reg_temp;

            if ((data_idx * 2) >= modbus_rx_byte_cnt)
                state = 5;
            else
                state = 3;

            break;

        case 5: // CRCµÍ×Ö½Ú
            crc_l = data;
            state = 6;
            break;

        case 6: // CRC¸ß×Ö½Ú
        {
            uint16_t crc_calc;
            uint16_t crc_recv;

            crc_h = data;
            crc_recv = crc_l | ((uint16_t)crc_h << 8);

            crc_calc = CRC16(modbus_raw_buf, 3 + modbus_rx_byte_cnt);

            if (crc_calc == crc_recv)
            {
                modbus_rx_frame_done = 1;

                for (uint8_t i = 0; i < modbus_reg_num; i++)
                {
                    modbus_date[i] = modbus_reg_buf[i];
                }
            }

            state = 0;
            raw_idx = 0;
            break;
        }

        default:
            state = 0;
            raw_idx = 0;
            break;
    }
}