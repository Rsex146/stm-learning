
#include <stdio.h>

extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
}

#include "imu_lib.h"


volatile uint32_t g_millis = 0;

void SysTick_Handler()
{
    ++g_millis;
}

void initLED()
{
     RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

     GPIO_InitTypeDef g;

     GPIO_StructInit(&g);
     g.GPIO_Pin = 0xFF00;
     g.GPIO_Mode = GPIO_Mode_OUT;
     g.GPIO_Speed = GPIO_Speed_Level_1;
     g.GPIO_OType = GPIO_OType_PP;
     g.GPIO_PuPd = GPIO_PuPd_NOPULL;
     GPIO_Init(GPIOE, &g);
}

void initButton()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitTypeDef g;

    GPIO_StructInit(&g);
    g.GPIO_Pin = GPIO_Pin_0;
    g.GPIO_Mode = GPIO_Mode_IN;
    g.GPIO_Speed = GPIO_Speed_Level_1;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &g);
}

void initSysTimer()
{
    RCC_ClocksTypeDef c;
    RCC_GetClocksFreq(&c);
    SysTick_Config(c.HCLK_Frequency / 1000);
}

void initUSB()
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

int main()
{
    bool usbMode = false;

    initLED();
    initButton();
    if (GPIOA->IDR & 0x1)
    {
        initUSB();
        GPIOE->ODR |= (1 << 9); // Indicated USB mode
        usbMode = true;
    }

    GPIOE->ODR |= (1 << 8); // Indicated init

    char buf[255];

    Quat qRef1, qRef2;

    I2C i2c;
    IMU imu1, imu2;
    i2c.init(I2C::Module::N1);
    imu1.init(&i2c, MPU6050::Module::N1);
    imu2.init(&i2c, MPU6050::Module::N2);

    GPIOE->ODR |= (1 << 10); // Indicate MPU init

    initSysTimer();

    while (true)
    {
        if (usbMode && g_millis > 16)
        {
            Quat q1 = imu1.read(g_millis);
            Quat q2 = imu2.read(g_millis);
            if ((GPIOA->IDR & 0x1))
            {
                qRef1 = q1.inverse();
                qRef2 = q2.inverse();
            }
            q1 = qRef1 * q1;
            q2 = qRef2 * q2;
            sprintf(buf, "{\"q1\":[%f,%f,%f,%f],\"q2\":[%f,%f,%f,%f]}\n",
                    q1[0], q1[1], q1[2], q1[3], q2[0], q2[1], q2[2], q2[3]);
            USB_Send_String(buf);
            g_millis = 0;
        }
    }

    return 0;
}
