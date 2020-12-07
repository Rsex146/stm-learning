
#ifndef __IMU_H__
#define __IMU_H__

#include "mpu6050_lib.h"
#include "MahonyAHRS.h"


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

/*class IMU
{
    MPU6050 m_mpu;
    Mahony m_filter;
    // unsigned long m_t;

public:
    IMU()
    {
        // m_t = 0;
    };

    bool init()
    {
        if (!m_mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
            return false;

        m_mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
        
        m_mpu.setIntFreeFallEnabled(false);  
        m_mpu.setIntZeroMotionEnabled(false);
        m_mpu.setIntMotionEnabled(false);
          
        m_mpu.setDHPFMode(MPU6050_DHPF_0_63HZ);
        m_mpu.setDLPFMode(MPU6050_DLPF_6);

        m_mpu.setMotionDetectionThreshold(8);
        m_mpu.setMotionDetectionDuration(5);

        m_mpu.setZeroMotionDetectionThreshold(4);
        m_mpu.setZeroMotionDetectionDuration(2);

        m_mpu.calibrateGyro(1000);

        // m_t = millis();

        return true;
    };

    Quat read()
    {
        Vector tGyro = m_mpu.readNormalizeGyro();
        Vector tAxel = m_mpu.readNormalizeAccel();

        // unsigned long t = millis();
        // float isf = (float)(t - m_t) / 1000.0f;
        // m_t = t;
        float isf = 0.02f;

        m_filter.updateIMU(tGyro.XAxis, tGyro.YAxis, tGyro.ZAxis, tAxel.XAxis, tAxel.YAxis, tAxel.ZAxis, isf);

        Quat q;
        m_filter.getQuaternion(q[0], q[1], q[2], q[3]);

        return q;
    };
};*/

#endif // __IMU_H__
