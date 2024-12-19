// BaseXライブラリ for M5Unified by Kikyoya(Tamotsu Kamigaki) (C) 2022-23.
// M5Unifiedバージョン 2023/11
/*
 * @Author: Sorzn
 * @Date: 2019-12-12 14:33:50
 * @LastEditTime: 2019-12-13 15:47:59
 * @Description: M5Stack project
 * @FilePath: /M5Stack/examples/Modules/BaseX/BaseX.h
 */

#ifndef __BASE_X_H__
#define __BASE_X_H__

// 独自のI2Cデバイスを使用(M5Unified)
#include "utility/I2C_Class.hpp"
using namespace m5;

#define BASE_X_ADDR (0x22)

#define BASE_X_SERVO_ANGLE_ADDR (0x00)
#define BASE_X_SERVO_PULSE_ADDR (0x10)
#define BASE_X_PWM_DUTY_ADDR    (0x20)
#define BASE_X_ENCODER_ADDR     (0x30)
#define BASE_X_SPEED_ADDR       (0x40)

/*
   |  0  |       1(3)  |      2 (1) |     3 (15) |    4, 5, 6, 7  |
   | mod |  position-p | position-i | position-d | position-point |
   
   |          8         |     9    |    10   |    11   |     12      | 
   | position-max-speed |  speed-p | speed-i | speed-d | speed-point | 
*/
#define BASE_X_CONFIG_ADDR      (0x50)

#define NORMAL_MODE     (0x00)
#define POSITION_MODE   (0x01)
#define SPEED_MODE      (0x02)


class BASE_X : public I2C_Device  // I2CDeviceを継承して独自のクラスを作成
{
    public:
        // デバイスの設定のため、コンストラクタを追加
        BASE_X(std::uint8_t i2c_addr = BASE_X_ADDR, std::uint32_t freq = 400000, m5::I2C_Class* i2c = &In_I2C)
          : I2C_Device ( i2c_addr, freq, i2c )
          {}  // 以下変更なし

        void SetMode(uint8_t pos, uint8_t mode);

        int32_t GetEncoderValue(uint8_t pos);
        void SetEncoderValue(uint8_t pos, int32_t encode);
        
        void SetMotorSpeed(uint8_t pos, int8_t duty);
        int8_t GetMotorSpeed(uint8_t pos);
        
        int8_t GetMotorSpeed20MS(uint8_t pos);
        
        void SetPositionPID(uint8_t pos, uint8_t kp, uint8_t ki, uint8_t kd);
        void SetPositionPoint(uint8_t pos, int32_t position_point);
        void SetPositionPIDMaxSpeed(uint8_t pos, uint8_t max_pwm);

        void SetSpeedPID(uint8_t pos, uint8_t kp, uint8_t ki, uint8_t kd);
        void SetSpeedPoint(uint8_t pos, int8_t speed_point);

        void SetServoAngle(uint8_t pos, uint8_t angle);
        void SetServoPulseWidth(uint8_t pos, uint16_t width);
    
    private:
        uint8_t CheckPos(uint8_t pos);
};

#define MAX(in, max) (((in) > (max)) ? (in) : (max))
#define MIN(in, min) (((in) < (min)) ? (in) : (min))
#define CONSTRAIN(in, min, max) MAX(min, MIN(in, max))

uint8_t BASE_X::CheckPos(uint8_t pos){  // 数をチェックしながら変換 1~4 => 0~3
    pos = pos - 1;
    pos = CONSTRAIN(pos, 0, 3);
    return pos;
}

/**
 * @description: 
 * @param pos: 1 ~ 4
 * @param mode: NORMAL, POSITION LOCK, SPEED LOCK
 * @return: None
 */
void BASE_X::SetMode(uint8_t pos, uint8_t mode){    
    pos = CheckPos(pos);
    //M5.I2C.writeByte(BASE_X_ADDR, BASE_X_CONFIG_ADDR + (0x10 * pos), mode);
    writeRegister8(BASE_X_CONFIG_ADDR + (0x10 * pos), mode);
}   

/**
 * @description: 
 * @param pos: 1 ~ 4
 * @return: encoder value
 */
int32_t BASE_X::GetEncoderValue(uint8_t pos){
    uint8_t addr;
    uint8_t read_buf[4] = {0, 0, 0, 0};
    
    pos = CheckPos(pos);
    addr = BASE_X_ENCODER_ADDR + 4*pos;
    //M5.I2C.readBytes(BASE_X_ADDR, addr, 4, read_buf);
    readRegister(addr, read_buf, 4);
    return (read_buf[0] << 24) | (read_buf[1] << 16) | (read_buf[2] << 8) | read_buf[3];
}

/**
 * @description: 
 * @param pos: 1 ~ 4 
 * @param encoder: INT32_MIN ~ INT32_MAX 
 * @return: None 
 */
void BASE_X::SetEncoderValue(uint8_t pos, int32_t encoder){
    uint8_t addr;
    uint8_t write_buf[4] = {0, 0, 0, 0};
    
    pos = CheckPos(pos);
    addr = BASE_X_ENCODER_ADDR + 4*pos;
    write_buf[0] = encoder >> 24;
    write_buf[1] = encoder >> 16;
    write_buf[2] = encoder >> 8;
    write_buf[3] = encoder & 0xff;
    
    //M5.I2C.writeBytes(BASE_X_ADDR, addr, write_buf, 4);
    writeRegister(addr, write_buf, 4);
}

/**
 * @description: 
 * @param pos: 1 ~ 4  
 * @param duty: Set motor speed, -127 ~ 127
 * @return: 
 */
void BASE_X::SetMotorSpeed(uint8_t pos, int8_t duty){
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_PWM_DUTY_ADDR + pos;
    
    //M5.I2C.writeByte(BASE_X_ADDR, addr, duty);
    writeRegister8(addr, duty);
}

/**
 * @description: 
 * @param pos: 1 ~ 4
 * @return Motor run speed, -127 ~ 127: 
 */
int8_t BASE_X::GetMotorSpeed(uint8_t pos){
    uint8_t read_data;
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_PWM_DUTY_ADDR + pos;
    
    //M5.I2C.readByte(BASE_X_ADDR, addr, &read_data);
    read_data = readRegister8(addr);
    return read_data;
}

/**
 * @description: 
 * @param pos: 1 ~ 4
 * @return: Motor encoder increments every 20 ms
 */
int8_t BASE_X::GetMotorSpeed20MS(uint8_t pos){
    uint8_t read_data;
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_SPEED_ADDR + pos;
    
    //M5.I2C.readByte(BASE_X_ADDR, addr, &read_data);
    read_data = readRegister8(addr);
    return read_data;
}

void BASE_X::SetPositionPID(uint8_t pos, uint8_t kp, uint8_t ki, uint8_t kd){
    uint8_t write_buf[3] = {0, 0, 0};
    uint8_t addr;
    pos = CheckPos(pos);
    
    addr = BASE_X_CONFIG_ADDR + pos * 0x10 + 0x01;
    write_buf[0] = kp;
    write_buf[1] = ki;
    write_buf[2] = kd;
    
    //M5.I2C.writeBytes(BASE_X_ADDR, addr, write_buf, 3);
    writeRegister(addr, write_buf, 3);
}   

/**
 * @description: 
 * @param pos: 1 ~ 4 
 * @param position_point: in POSITION mode, motor will lock in this value, INT32_MIN ~ INT32_MAX
 * @return: None
 */
void BASE_X::SetPositionPoint(uint8_t pos, int32_t position_point){
    uint8_t addr;
    uint8_t write_buf[4] = {0, 0, 0, 0};
    
    pos = CheckPos(pos);
    addr = BASE_X_CONFIG_ADDR + pos * 0x10 + 0x04;
    write_buf[0] = position_point & 0xff;
    write_buf[1] = position_point >> 8;
    write_buf[2] = position_point >> 16;
    write_buf[3] = position_point >> 24;
    
    // Serial.printf(" %x %x %x %x \r\n", write_buf[0], write_buf[1], write_buf[2], write_buf[3]);
    //M5.I2C.writeBytes(BASE_X_ADDR, addr, write_buf, 4);
    writeRegister(addr, write_buf, 4);
}

/**
 * @description: 
 * @param  pos: 1 ~ 4  
 * @param  max_pwm: 0 ~ 127, POSITION mode, max speed  
 * @return: 
 */
void BASE_X::SetPositionPIDMaxSpeed(uint8_t pos, uint8_t max_pwm){
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_CONFIG_ADDR + pos * 0x10 + 0x08;
    
    //M5.I2C.writeByte(BASE_X_ADDR, addr, max_pwm);
    writeRegister8(addr, max_pwm);
}

void BASE_X::SetSpeedPID(uint8_t pos, uint8_t kp, uint8_t ki, uint8_t kd){
    uint8_t write_buf[3] = {0, 0, 0};
    uint8_t addr;
    pos = CheckPos(pos);
    
    addr = BASE_X_CONFIG_ADDR + pos * 0x10 + 0x09;
    write_buf[0] = kp;
    write_buf[1] = ki;
    write_buf[2] = kd;
    
    //M5.I2C.writeBytes(BASE_X_ADDR, addr, write_buf, 3);
    writeRegister(addr, write_buf, 3);
}

/**
 * @description: 
 * @param pos: 1 ~ 4 
 * @param speed_point: speed_point is lock GetMotorSpeed20MS(), not GetMotorSpeed(), maybe -20 ~ 20
 * @return: None
 */
void BASE_X::SetSpeedPoint(uint8_t pos, int8_t speed_point){
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_CONFIG_ADDR + pos * 0x10 + 0x0c;
    
    //M5.I2C.writeByte(BASE_X_ADDR, addr, (uint8_t)speed_point);
    writeRegister8(addr, (uint8_t)speed_point);
}

/**
 * @description: 
 * @param pos: 1 ~ 2
 * @param angle: 0 ~ 180
 * @return: None
 */
void BASE_X::SetServoAngle(uint8_t pos, uint8_t angle){
    uint8_t addr;
    pos = CheckPos(pos);
    addr = BASE_X_SERVO_ANGLE_ADDR + pos;
    
    //M5.I2C.writeByte(BASE_X_ADDR, addr, angle);
    writeRegister8(addr, angle);
}

/**
 * @description: 
 * @param pos: 1 ~ 2
 * @param width: 500 ~ 2500
 * @return: None
 */
void BASE_X::SetServoPulseWidth(uint8_t pos, uint16_t width){
    uint8_t addr;
    uint8_t write_buf[2] = {0, 0};
    
    pos = CheckPos(pos);
    addr = BASE_X_SERVO_PULSE_ADDR + pos * 0x02;
    write_buf[0] = width >> 8;
    write_buf[1] = width & 0xff;
    //M5.I2C.writeBytes(BASE_X_ADDR, addr, write_buf, 2);
    writeRegister(addr, write_buf, 2);
}

#endif
