
#ifndef __MPU6050_LIB_H__
#define __MPU6050_LIB_H__

#include "i2c_lib.h"


class MPU6050
{
public:
    enum Module
    {
        N1 = 0x68,
        N2 = 0x69
    };

private:
    I2C *m_i2c;
    uint32_t m_address;
    float m_threshold[3];

    void delay(uint32_t ms) const;
    void readRawGyro(int16_t *gyro);
    void readRawAccel(int16_t *accel);

public:
    MPU6050();

    bool init(Module gyroAcelModule, I2C *i2c);
    void calibrate(uint16_t sampleCount, uint16_t &dmaSrc);

    void readGyro(float *gyro);
    void readAccel(float *accel);
};

#endif // __MPU6050_LIB_H__
