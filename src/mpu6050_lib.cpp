
#include "mpu6050_lib.h"
#include "timer_lib.h"


#define MPU6050_REG_ACCEL_XOFFS_H     (0x06)
#define MPU6050_REG_ACCEL_XOFFS_L     (0x07)
#define MPU6050_REG_ACCEL_YOFFS_H     (0x08)
#define MPU6050_REG_ACCEL_YOFFS_L     (0x09)
#define MPU6050_REG_ACCEL_ZOFFS_H     (0x0A)
#define MPU6050_REG_ACCEL_ZOFFS_L     (0x0B)
#define MPU6050_REG_GYRO_XOFFS_H      (0x13)
#define MPU6050_REG_GYRO_XOFFS_L      (0x14)
#define MPU6050_REG_GYRO_YOFFS_H      (0x15)
#define MPU6050_REG_GYRO_YOFFS_L      (0x16)
#define MPU6050_REG_GYRO_ZOFFS_H      (0x17)
#define MPU6050_REG_GYRO_ZOFFS_L      (0x18)
#define MPU6050_REG_CONFIG            (0x1A)
#define MPU6050_REG_GYRO_CONFIG       (0x1B)
#define MPU6050_REG_ACCEL_CONFIG      (0x1C)
#define MPU6050_REG_FF_THRESHOLD      (0x1D)
#define MPU6050_REG_FF_DURATION       (0x1E)
#define MPU6050_REG_MOT_THRESHOLD     (0x1F)
#define MPU6050_REG_MOT_DURATION      (0x20)
#define MPU6050_REG_ZMOT_THRESHOLD    (0x21)
#define MPU6050_REG_ZMOT_DURATION     (0x22)
#define MPU6050_REG_INT_PIN_CFG       (0x37)
#define MPU6050_REG_INT_ENABLE        (0x38)
#define MPU6050_REG_INT_STATUS        (0x3A)
#define MPU6050_REG_ACCEL_XOUT_H      (0x3B)
#define MPU6050_REG_ACCEL_XOUT_L      (0x3C)
#define MPU6050_REG_ACCEL_YOUT_H      (0x3D)
#define MPU6050_REG_ACCEL_YOUT_L      (0x3E)
#define MPU6050_REG_ACCEL_ZOUT_H      (0x3F)
#define MPU6050_REG_ACCEL_ZOUT_L      (0x40)
#define MPU6050_REG_TEMP_OUT_H        (0x41)
#define MPU6050_REG_TEMP_OUT_L        (0x42)
#define MPU6050_REG_GYRO_XOUT_H       (0x43)
#define MPU6050_REG_GYRO_XOUT_L       (0x44)
#define MPU6050_REG_GYRO_YOUT_H       (0x45)
#define MPU6050_REG_GYRO_YOUT_L       (0x46)
#define MPU6050_REG_GYRO_ZOUT_H       (0x47)
#define MPU6050_REG_GYRO_ZOUT_L       (0x48)
#define MPU6050_REG_MOT_DETECT_STATUS (0x61)
#define MPU6050_REG_MOT_DETECT_CTRL   (0x69)
#define MPU6050_REG_USER_CTRL         (0x6A)
#define MPU6050_REG_PWR_MGMT_1        (0x6B)
#define MPU6050_REG_WHO_AM_I          (0x75)
#define MPU6050_REG_WHO_AM_I_EXPECTED (0x68)

#define READ_REG8(reg) \
    m_i2c->read(m_address, reg, &value, 1);

#define WRITE_REG8(reg) \
    m_i2c->write(m_address, reg, &value, 1);

#define SET_REG_BIT(reg, bit) \
    READ_REG8(reg);           \
    value |= (1 << (bit));    \
    WRITE_REG8(reg);

#define RESET_REG_BIT(reg, bit) \
    READ_REG8(reg);             \
    value &= ~(1 << (bit));     \
    WRITE_REG8(reg);

MPU6050::MPU6050()
{
    m_i2c = nullptr;
    m_threshold[0] = m_threshold[1] = m_threshold[2] = 0.0f;
}

bool MPU6050::init(Module gyroAcelModule, I2C *i2c)
{
    uint8_t value;

    m_i2c = i2c;
    m_address = (uint32_t)gyroAcelModule;

    READ_REG8(MPU6050_REG_WHO_AM_I);
    if (value != MPU6050_REG_WHO_AM_I_EXPECTED)
        return false;

    // Disable sleep mode and set clock source to X.
    READ_REG8(MPU6050_REG_PWR_MGMT_1);
    value &= ~(1 << 6); // Disable sleep.
    value &= 0b11111000; // Cleanup clock.
    value |= 0b001; // Set clock to X.
    WRITE_REG8(MPU6050_REG_PWR_MGMT_1);

    // Set gyro scale to 2000 DPS.
    READ_REG8(MPU6050_REG_GYRO_CONFIG);
    value |= 0b00011000;
    WRITE_REG8(MPU6050_REG_GYRO_CONFIG);

    // Set accel range to 2G.
    READ_REG8(MPU6050_REG_ACCEL_CONFIG);
    value &= 0b11100111;
    WRITE_REG8(MPU6050_REG_ACCEL_CONFIG);

    // Set accel power-on delay to 3 ms.
    READ_REG8(MPU6050_REG_MOT_DETECT_CTRL);
    value |= 0b00110000;
    WRITE_REG8(MPU6050_REG_MOT_DETECT_CTRL);

    // Disable free fall interrupt, zero motion interrupt and motion interrupt.
    READ_REG8(MPU6050_REG_INT_ENABLE);
    value &= 0b00011111;
    WRITE_REG8(MPU6050_REG_INT_ENABLE);

    // Set high-pass filter of accel to 63 Hz.
    READ_REG8(MPU6050_REG_ACCEL_CONFIG);
    value &= 0b11111000; // Cleanup filter.
    value |= 0b100; // Set filter to 63 Hz.
    WRITE_REG8(MPU6050_REG_ACCEL_CONFIG);

    // Set low-pass filter to function 6.
    READ_REG8(MPU6050_REG_CONFIG);
    value &= 0b11111000; // Cleanup filter.
    value |= 0b110; // Set filter to function 6.
    WRITE_REG8(MPU6050_REG_CONFIG);

    // Set motion detection threshold.
    value = 8;
    WRITE_REG8(MPU6050_REG_MOT_THRESHOLD);

    // Set motion detection duration.
    value = 5;
    WRITE_REG8(MPU6050_REG_MOT_DURATION);

    // Set zero motion detection threshold.
    value = 4;
    WRITE_REG8(MPU6050_REG_ZMOT_THRESHOLD);

    // Set zero motion detection duration.
    value = 2;
    WRITE_REG8(MPU6050_REG_ZMOT_DURATION);

    return true;
}

void MPU6050::calibrate(uint16_t sampleCount)
{
    int32_t sum[3] = { 0, 0, 0 };
    int16_t gyro[3];
    GPIOE->ODR = 0;
    uint16_t interval = sampleCount / 8;
    uint8_t led = 0;
    for (uint16_t i = 0; i < sampleCount; ++i)
    {
        if (i % interval == 0)
        {
            GPIOE->ODR |= (1 << (led + 8));
            ++led;
        }
        readRawGyro(gyro);
        for (uint8_t j = 0; j < 3; ++j)
        {
            sum[j] += (int32_t)gyro[j];
            g_timer.wait(5);
        }
    }
    for (uint8_t i = 0; i < 3; ++i)
        m_threshold[i] = (float)sum[i] / (float)sampleCount;
    GPIOE->ODR = 0xFF00;
}

void MPU6050::readGyro(float *gyro)
{
    const float dpsPerDigit = 0.060975f; // 2000 DPS.
    int16_t raw[3];
    readRawGyro(raw);
    for (uint8_t i = 0; i < 3; ++i)
        gyro[i] = ((float)raw[i] - m_threshold[i]) * dpsPerDigit;
}

void MPU6050::readAccel(float *accel)
{
    const float rangePerDigit = 0.000061f; // 2G.
    int16_t raw[3];
    readRawAccel(raw);
    for (uint8_t i = 0; i < 3; ++i)
        accel[i] = (float)raw[i] * rangePerDigit * 9.80665f;
}

void MPU6050::readRawGyro(int16_t *gyro)
{
    uint8_t buffer[6];
    m_i2c->read(m_address, MPU6050_REG_GYRO_XOUT_H, buffer, 6);
    for (uint8_t i = 0; i < 3; ++i)
        gyro[i] = (((int16_t)buffer[i << 1]) << 8) | (int16_t)buffer[(i << 1) + 1];
}

void MPU6050::readRawAccel(int16_t *accel)
{
    uint8_t buffer[6];
    m_i2c->read(m_address, MPU6050_REG_ACCEL_XOUT_H, buffer, 6);
    for (uint8_t i = 0; i < 3; ++i)
        accel[i] = (((int16_t)buffer[i << 1]) << 8) | (int16_t)buffer[(i << 1) + 1];
}
