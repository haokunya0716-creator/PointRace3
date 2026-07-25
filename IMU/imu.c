#include "imu.h"
#include "clock.h"
#include "ti_msp_dl_config.h"
//实例化数据结构体
struct SAngle Angle;
struct SGyro Gyro;
struct SAccel Accel;
struct SQuat Quat;
/**
 * @brief 三个辅助函数
 */
static void IMU_send_char(char ch)
{
    while( DL_UART_isBusy(IMU_INST) == true );
    DL_UART_Main_transmitData(IMU_INST, ch);
}

static void IMU_send_bytes(uint8_t* data, uint32_t len)
{
    for(uint32_t i = 0; i < len; i++)
    {
        IMU_send_char(data[i]);
    }
}
//串口发送字符串
static void IMU_send_string(char* str)
{
    //当前字符串地址不在结尾 并且 字符串首地址不为空
    while(*str!=0&&str!=0)
    {
        //发送字符串首地址中的字符，并且在发送完成之后首地址自增
       IMU_send_char(*str++);
    }
}
//初始化IMU（开启中断）                                 
void IMU_Init(void)
{

    NVIC_SetPriority(IMU_INST_INT_IRQN, 1);

    // 清理上电期间或 delay_ms 期间积攒的垃圾数据和溢出报错
    DL_UART_clearInterruptStatus(IMU_INST, 0xFFFFFFFF); // 清除所有潜在的错误标志
    while (DL_UART_isRXFIFOEmpty(IMU_INST) == false) {
        DL_UART_Main_receiveData(IMU_INST); 
    }

    NVIC_ClearPendingIRQ(IMU_INST_INT_IRQN);
    NVIC_EnableIRQ(IMU_INST_INT_IRQN);
    
    // 初始化时校准偏置
    //performCaliBias();
    
    // Z轴角度归零
    sendCaliYawCommand1(); 
		//sendCaliYawCommand6(); 
}


/******************************************************************************
 * 单轴陀螺仪数据解析函数：接收0x5A开头的5字节数据帧
******************************************************************************/
void CopeSeriaIMU1Data(unsigned char ucData)
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

        Gyro.wz    = (float)wz    / 32768.0f * 2000.0f;
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
        Angle.Yaw = (float)rawYaw / 32768.0f * 180.0f;
    }
    ucRxCnt = 0;
}
/******************************************************************************
 * 六轴陀螺仪数据解析函数：接收0x5A开头的5字节数据帧
******************************************************************************/
void CopeSeriaIMU6Data(unsigned char ucData)
{
    static unsigned char ucRxBuffer[11];
    static unsigned char ucRxCnt = 0;
    unsigned char sum = 0;
    int i;

    // 缓存数据
    ucRxBuffer[ucRxCnt++] = ucData;

    // 帧头校验
    if (ucRxBuffer[0] != 0x5A)
    {
        ucRxCnt = 0;
        return;
    }

    // 帧类型判断，确定帧长度
    // 加速度/角速度/角度帧: 11字节 (0x5A + TYPE + 4组数据 + SUM)
    // 四元数帧: 11字节 (0x5A + TYPE + 4组数据 + SUM)
    // 寄存器读报包: 11字节
    if (ucRxCnt < 11) return;  // 等待完整帧

    // 根据TYPE计算校验和
    switch (ucRxBuffer[1])
    {
        case 0xAA:  // 角速度
            sum = ucRxBuffer[0] + ucRxBuffer[1] +
                  ucRxBuffer[2] + ucRxBuffer[3] +   // WxL, WxH
                  ucRxBuffer[4] + ucRxBuffer[5] +   // WyL, WyH
                  ucRxBuffer[6] + ucRxBuffer[7] +   // WzL, WzH
                  ucRxBuffer[8] + ucRxBuffer[9];    // 0x00, 0x00
            
            if (sum != ucRxBuffer[10])
            {
                ucRxCnt = 0;
                return;
            }
            
            // 解析角速度
            {
                short wx = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);
                short wy = (short)((ucRxBuffer[5] << 8) | ucRxBuffer[4]);
                short wz = (short)((ucRxBuffer[7] << 8) | ucRxBuffer[6]);
                
                Gyro.wx = (float)wx / 32768.0f * 2000.0f;  // °/s
                Gyro.wy = (float)wy / 32768.0f * 2000.0f;
                Gyro.wz = (float)wz / 32768.0f * 2000.0f;
            }
            break;
            
        case 0xBB:  // 角度
            sum = ucRxBuffer[0] + ucRxBuffer[1] +
                  ucRxBuffer[2] + ucRxBuffer[3] +   // RollL, RollH
                  ucRxBuffer[4] + ucRxBuffer[5] +   // PitchL, PitchH
                  ucRxBuffer[6] + ucRxBuffer[7] +   // YawL, YawH
                  ucRxBuffer[8] + ucRxBuffer[9];    // 0x00, 0x00
            
            if (sum != ucRxBuffer[10])
            {
                ucRxCnt = 0;
                return;
            }
            
            // 解析角度
            {
                short roll  = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);
                short pitch = (short)((ucRxBuffer[5] << 8) | ucRxBuffer[4]);
                short yaw   = (short)((ucRxBuffer[7] << 8) | ucRxBuffer[6]);
                
                Angle.Roll  = (float)roll  / 32768.0f * 180.0f;  // °
                Angle.Pitch = (float)pitch / 32768.0f * 180.0f;
                Angle.Yaw   = (float)yaw   / 32768.0f * 180.0f;
            }
            break;
            
        case 0xCC:  // 加速度
            sum = ucRxBuffer[0] + ucRxBuffer[1] +
                  ucRxBuffer[2] + ucRxBuffer[3] +   // AxL, AxH
                  ucRxBuffer[4] + ucRxBuffer[5] +   // AyL, AyH
                  ucRxBuffer[6] + ucRxBuffer[7] +   // AzL, AzH
                  ucRxBuffer[8] + ucRxBuffer[9];    // 0x00, 0x00
            
            if (sum != ucRxBuffer[10])
            {
                ucRxCnt = 0;
                return;
            }
            
            // 解析加速度
            {
                short ax = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);
                short ay = (short)((ucRxBuffer[5] << 8) | ucRxBuffer[4]);
                short az = (short)((ucRxBuffer[7] << 8) | ucRxBuffer[6]);
                
                const float G = 9.8f;  // 重力加速度
                Accel.ax = (float)ax / 32768.0f * 16.0f * G;  // m/s²
                Accel.ay = (float)ay / 32768.0f * 16.0f * G;
                Accel.az = (float)az / 32768.0f * 16.0f * G;
            }
            break;
            
        case 0xDD:  // 四元数
            sum = ucRxBuffer[0] + ucRxBuffer[1] +
                  ucRxBuffer[2] + ucRxBuffer[3] +   // Q0L, Q0H
                  ucRxBuffer[4] + ucRxBuffer[5] +   // Q1L, Q1H
                  ucRxBuffer[6] + ucRxBuffer[7] +   // Q2L, Q2H
                  ucRxBuffer[8] + ucRxBuffer[9];    // Q3L, Q3H
            
            if (sum != ucRxBuffer[10])
            {
                ucRxCnt = 0;
                return;
            }
            
            // 解析四元数
            {
                short q0 = (short)((ucRxBuffer[3] << 8) | ucRxBuffer[2]);
                short q1 = (short)((ucRxBuffer[5] << 8) | ucRxBuffer[4]);
                short q2 = (short)((ucRxBuffer[7] << 8) | ucRxBuffer[6]);
                short q3 = (short)((ucRxBuffer[9] << 8) | ucRxBuffer[8]);
                
                Quat.q0 = (float)q0 / 32768.0f;
                Quat.q1 = (float)q1 / 32768.0f;
                Quat.q2 = (float)q2 / 32768.0f;
                Quat.q3 = (float)q3 / 32768.0f;
            }
            break;
            
        case 0xEE:  // 寄存器读报包（可根据需要解析）
            // 寄存器读报包的格式与上述类似，可按需处理
            sum = ucRxBuffer[0] + ucRxBuffer[1] +
                  ucRxBuffer[2] + ucRxBuffer[3] +
                  ucRxBuffer[4] + ucRxBuffer[5] +
                  ucRxBuffer[6] + ucRxBuffer[7] +
                  ucRxBuffer[8] + ucRxBuffer[9];
            
            if (sum != ucRxBuffer[10])
            {
                ucRxCnt = 0;
                return;
            }
            // 寄存器数据读取处理
            break;
            
        default:
            // 未知类型，复位
            ucRxCnt = 0;
            return;
    }
    
    // 解析成功，复位接收计数器
    ucRxCnt = 0;
}

// 获取yaw轴角度
float getYaw(void)
{
    return Angle.Yaw;
}

// 获取Pitch轴角度
float getPitch(void)
{
    return Angle.Pitch;
}

// 获取Roll轴角度
float getRoll(void)
{
    return Angle.Roll;
}

// 获取x轴角加速度
float getGyroX(void)
{
    return Gyro.wx;
}

// 获取y轴角加速度
float getGyroY(void)
{
    return Gyro.wy;
}

// 获取z轴角加速度
float getGyroZ(void)
{
    return Gyro.wz;
}



/********************* IMU 写格式  ************************/
uint8_t Key[5]       = {0x55, 0xAA, 0x13, 0x8E, 0x5F}; // 解锁指令（解锁，往该寄存器写0x8E5F（其他值无效））
uint8_t Yaw1_Zero[5]  = {0x55, 0xAA, 0x15, 0x00, 0x00}; // 单轴Z轴归零指令
uint8_t Yaw6_Zero[5]  = {0x55, 0xAA, 0x0A, 0x04, 0x00}; // 单轴Z轴归零指令
uint8_t Save[5]      = {0x55, 0xAA, 0x00, 0x00, 0x00}; // 保存指令
uint8_t BIAS_CAL[5]  = {0x55, 0xAA, 0x0A, 0x01, 0x00}; // 获取零偏指令


/******************************************************************************
 * 发送 单轴Z轴角度归零命令
 ******************************************************************************/
void sendCaliYawCommand1(void)
{
    IMU_send_bytes(Key, 5);
    delay_ms(100);
    IMU_send_bytes(Yaw1_Zero, 5);
    delay_ms(100);
    IMU_send_bytes(Save, 5);
}


/******************************************************************************
 * 发送 六轴Z轴角度归零命令
 ******************************************************************************/
void sendCaliYawCommand6(void)
{
    IMU_send_bytes(Key, 5);
    delay_ms(100);
    IMU_send_bytes(Yaw6_Zero, 5);
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

////printf函数重定义
//int fputc(int ch, FILE *stream)
//{
//    //当串口0忙的时候等待，不忙的时候再发送传进来的字符
//   while( DL_UART_isBusy(user_INST) == true );

//   DL_UART_Main_transmitData(user_INST, ch);

//   return ch;
//}


// 延时函数封装映射
void delay_ms(int __ms)  { mspm0_delay_ms(__ms); }
void delay_us(int __us)  { delay_cycles((CPUCLK_FREQ / 1000000) * __us); }