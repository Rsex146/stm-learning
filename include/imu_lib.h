
#ifndef __IMU_H__
#define __IMU_H__

#include "mpu6050_lib.h"
#include "MahonyAHRS.h"
#include "timer_lib.h"


class Quat
{
    float m_coords[4];

public:
    Quat()
    {
        m_coords[0] = 0.0;
        m_coords[1] = 0.0;
        m_coords[2] = 0.0;
        m_coords[3] = 1.0;
    };

    Quat(float x, float y, float z, float w)
    {
        m_coords[0] = x;
        m_coords[1] = y;
        m_coords[2] = z;
        m_coords[3] = w;
    };

    Quat inverse() const
    {
        float norm = m_coords[0] * m_coords[0] + m_coords[1] * m_coords[1] + m_coords[2] * m_coords[2] + m_coords[3] * m_coords[3];
        return Quat(-m_coords[0] / norm, -m_coords[1] / norm, -m_coords[2] / norm, m_coords[3] / norm);
    };

    void normalize()
    {
        float magn = Mahony::invSqrt(m_coords[0] * m_coords[0] + m_coords[1] * m_coords[1] + m_coords[2] * m_coords[2] + m_coords[3] * m_coords[3]);
        for (int i = 0; i < 4; ++i)
            m_coords[i] *= magn;
    };

    void operator *=(const Quat &q)
    {
        Quat result = *this * q;
        for (int i = 0; i < 4; ++i)
            m_coords[i] = result[i];
    };
    Quat operator *(const Quat &q) const
    {
        return Quat(m_coords[0] * q[3] + m_coords[1] * q[2] - m_coords[2] * q[1] + m_coords[3] * q[0],
                    -m_coords[0] * q[2] + m_coords[1] * q[3] + m_coords[2] * q[0] + m_coords[3] * q[1],
                    m_coords[0] * q[1] - m_coords[1] * q[0] + m_coords[2] * q[3] + m_coords[3] * q[2],
                    -m_coords[0] * q[0] - m_coords[1] * q[1] - m_coords[2] * q[2] + m_coords[3] * q[3]);
    };

    float operator [](int index) const
    {
        return m_coords[index];
    };
    float &operator [](int index)
    {
        return m_coords[index];
    };
};

class IMU
{
    MPU6050 m_mpu;
    Mahony m_filter;

public:
    IMU()
    {
    };

    bool init(I2C *i2c, MPU6050::Module module)
    {
        if (!m_mpu.init(module, i2c))
            return false;

        return true;
    };

    void calibrate(uint16_t &dmaSrc)
    {
        m_mpu.calibrate(50, dmaSrc);
    };

    Quat read()
    {
        float gyro[3];
        float accel[3];
        m_mpu.readGyro(gyro);
        m_mpu.readAccel(accel);

        float isf = (float)g_timer.millis() / 1000.0f;

        m_filter.updateIMU(gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2], isf);

        Quat q;
        m_filter.getQuaternion(q[0], q[1], q[2], q[3]);

        return q;
    };
};

#endif // __IMU_H__
