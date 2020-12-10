
#include <stdio.h>

extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
}

#include "imu_lib.h"
#include "timer_lib.h"
#include "stm32f30x_dma.h"


uint16_t g_dmaSrc = 0;

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

void initUSB()
{
    Set_System();
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

void initDMA()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef d;
    DMA_StructInit(&d);
    d.DMA_PeripheralBaseAddr = (uint32_t)&g_dmaSrc;
    d.DMA_MemoryBaseAddr = (uint32_t)&(GPIOE->ODR);
    d.DMA_DIR = DMA_DIR_PeripheralSRC;
    d.DMA_BufferSize = DMA_PeripheralDataSize_HalfWord;
    d.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    d.DMA_MemoryInc = DMA_MemoryInc_Disable;
    d.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    d.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    d.DMA_Mode = DMA_Mode_Circular;
    d.DMA_Priority = DMA_Priority_Medium;
    d.DMA_M2M = DMA_M2M_Enable;
    DMA_Init(DMA1_Channel1, &d);
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

int main()
{
    bool usbMode = false;

    initLED();
    initButton();
    initDMA();
    if (GPIOA->IDR & 0x1)
    {
        initUSB();
        g_dmaSrc |= (1 << 9); // Indicated USB mode
        usbMode = true;
    }

    g_dmaSrc |= (1 << 8); // Indicated init

    char buf[255];

    Quat qRef1, qRef2;

    I2C i2c;
    IMU imu1, imu2;
    i2c.init(I2C::Module::N1);
    imu1.init(&i2c, MPU6050::Module::N1);
    imu2.init(&i2c, MPU6050::Module::N2);

    g_dmaSrc |= (1 << 10); // Indicate MPU init

    g_timer.init();

    while (true)
    {
        if (usbMode && g_timer.millis() > 16)
        {
            Quat q1, q2;
            if ((GPIOA->IDR & 0x1))
            {
                imu1.calibrate(g_dmaSrc);
                imu2.calibrate(g_dmaSrc);
                q1 = imu1.read();
                q2 = imu2.read();
                qRef1 = q1.inverse();
                qRef2 = q2.inverse();
            }
            else
            {
                q1 = imu1.read();
                q2 = imu2.read();
            }
            q1 = qRef1 * q1;
            q2 = qRef2 * q2;
            sprintf(buf, "{\"q1\":[%f,%f,%f,%f],\"q2\":[%f,%f,%f,%f]}\n",
                    q1[0], q1[1], q1[2], q1[3], q2[0], q2[1], q2[2], q2[3]);
            USB_Send_String(buf);
            g_timer.reset();
        }
    }

    return 0;
}
