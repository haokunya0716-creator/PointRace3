#ifndef MOTOR_READ_ENC_h
#define MOTOR_READ_ENC_h

#include "ti_msp_dl_config.h"
#include "motor_crc.h"

extern volatile int16_t modbus_date[8];
extern volatile uint8_t modbus_rx_frame_done;

void Modbus_ParseFrame(uint8_t data);
void Modbus_ParseFrame1(uint8_t data);


#endif
