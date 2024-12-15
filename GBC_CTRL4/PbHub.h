#include "Adafruit_I2CDevice.h" // 何故かこれがないとSPIライブラリでエラーが出る　？？？
#ifndef __PORTHUB_H__
#define __PORTHUB_H__

#include <Wire.h>

#define IIC_ADDR1 0x61
#define IIC_ADDR2 0x62
#define IIC_ADDR3 0x63
#define IIC_ADDR4 0x64
#define IIC_ADDR5 0x65
#define IIC_ADDR6 0x66
#define IIC_ADDR7 0x67
#define IIC_ADDR8 0x68
#define HUB1_ADDR 0x40
#define HUB2_ADDR 0x50
#define HUB3_ADDR 0x60
#define HUB4_ADDR 0x70
#define HUB5_ADDR 0x80
#define HUB6_ADDR 0xA0

class PortHub {
   public:
    PortHub();
    PortHub(uint8_t iic_addr);
    void begin();

    uint16_t hub_a_read_value(uint8_t reg);

    uint8_t hub_d_read_value_A(uint8_t reg);
    uint8_t hub_d_read_value_B(uint8_t reg);

    void hub_d_wire_value_A(uint8_t reg, uint16_t level);
    void hub_d_wire_value_B(uint8_t reg, uint16_t level);

    void hub_a_wire_value_A(uint8_t reg, uint16_t duty);
    void hub_a_wire_value_B(uint8_t reg, uint16_t duty);

    void hub_wire_length(uint8_t reg, uint16_t length);

    void hub_wire_index_color(uint8_t reg, uint16_t num, uint8_t r, int8_t g,
                              uint8_t b);

    void hub_wire_fill_color(uint8_t reg, uint16_t first, uint16_t count,
                             uint8_t r, int8_t g, uint8_t b);

    void hub_wire_setBrightness(uint8_t reg, uint8_t brightness);

   public:
   private:
    uint8_t _iic_addr = IIC_ADDR1;

   private:
};


PortHub::PortHub() {
}

PortHub::PortHub(uint8_t iic_addr) {
    _iic_addr = iic_addr;
}

void PortHub::begin() {
  Wire.begin(M5.Ex_I2C.getSDA(), M5.Ex_I2C.getSCL());
//    Wire.begin();
}

uint16_t PortHub::hub_a_read_value(uint8_t reg) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x06);
    Wire.endTransmission();

    uint8_t RegValue_L, RegValue_H;

    Wire.requestFrom(_iic_addr, (uint8_t)2);
    while (Wire.available()) {
        RegValue_L = Wire.read();
        RegValue_H = Wire.read();
    }

    return (RegValue_H << 8) | RegValue_L;
}

uint8_t PortHub::hub_d_read_value_A(uint8_t reg) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x04);
    Wire.endTransmission();

    uint8_t RegValue;

    Wire.requestFrom(_iic_addr, (uint8_t)1);
    while (Wire.available()) {
        RegValue = Wire.read();
    }
    return RegValue;
}

uint8_t PortHub::hub_d_read_value_B(uint8_t reg) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x05);
    Wire.endTransmission();

    uint8_t RegValue;

    Wire.requestFrom(_iic_addr, (uint8_t)1);
    while (Wire.available()) {
        RegValue = Wire.read();
    }
    return RegValue;
}

void PortHub::hub_d_wire_value_A(uint8_t reg, uint16_t level) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x00);
    Wire.write(level & 0xff);
    Wire.endTransmission();
}

void PortHub::hub_d_wire_value_B(uint8_t reg, uint16_t level) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x01);
    Wire.write(level & 0xff);
    Wire.endTransmission();
}

void PortHub::hub_a_wire_value_A(uint8_t reg, uint16_t duty) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x02);
    Wire.write(duty & 0xff);
    Wire.endTransmission();
}

void PortHub::hub_a_wire_value_B(uint8_t reg, uint16_t duty) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x03);
    Wire.write(duty & 0xff);
    Wire.endTransmission();
}

void PortHub::hub_wire_length(uint8_t reg, uint16_t length) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x08);
    Wire.write(length & 0xff);
    Wire.write(length >> 8);
    Wire.endTransmission();
}
// バグがあってSK6812ではLEDの位置は指定不可、必ず0番になる
void PortHub::hub_wire_index_color(uint8_t reg, uint16_t num, uint8_t r,
                                   int8_t g, uint8_t b) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x09);
    Wire.write(num & 0xff);
    Wire.write(num >> 8);
    Wire.write(r);
    Wire.write(g);
    Wire.write(b);
    Wire.endTransmission();
}

void PortHub::hub_wire_fill_color(uint8_t reg, uint16_t first, uint16_t count,
                                  uint8_t r, int8_t g, uint8_t b) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x0a);
    Wire.write(first & 0xff);
    Wire.write(first >> 8);

    Wire.write(count & 0xff);
    Wire.write(count >> 8);

    Wire.write(r);
    Wire.write(g);
    Wire.write(b);
    Wire.endTransmission();
}

void PortHub::hub_wire_setBrightness(uint8_t reg, uint8_t brightness) {
    Wire.beginTransmission(_iic_addr);
    Wire.write(reg | 0x0b);
    Wire.write(brightness & 0xff);
    Wire.endTransmission();
}

#endif
