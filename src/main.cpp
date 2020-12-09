
#include <stdio.h>

extern "C" {
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
}

#include "imu_lib.h"


bool g_usb = false;

void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

void led()
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

void button()
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

// int main()
// {
// 	Set_System();
// 	Set_USBClock();
// 	USB_Interrupts_Config();
// 	USB_Init();

// 	led();
// 	GPIOE->ODR = (1 << 8);

// 	while (1)
// 	{
// 		USB_Send_Data(65);
// 		//__NOP();
// 	}

// 	return 0;
// }

void init_usb()
{
	Set_System();
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
}

void button_scan()
{
	static uint8_t f = 0;
	if ((GPIOA->IDR & 0x1) && (f == 0))
	{
		myDelay(50000);
		if ((GPIOA->IDR & 0x1) && (f == 0))
		{
			init_usb();
			GPIOE->ODR = (1 << 8);
			g_usb = true;
			f = 1;
		}
	}
	if (!(GPIOA->IDR & 0x1) && (f == 1))
	{
		f = 0;
	}
}

bool button_scan2()
{
	static uint8_t f = 0;
	if ((GPIOA->IDR & 0x1) && (f == 0))
	{
		myDelay(50000);
		if ((GPIOA->IDR & 0x1) && (f == 0))
		{
			f = 1;
			return true;
		}
	}
	if (!(GPIOA->IDR & 0x1) && (f == 1))
	{
		f = 0;
	}
	return false;
}

int main()
{
	led();
	button();

	GPIOE->ODR = (1 << 9);

	button_scan();

	/*uint8_t who;
	I2C i2c;
	i2c.init(I2C::Module::N1);
	i2c.read(0x68, 0x75, &who, 1);*/

	char buf[255];

	Quat qRef1, qRef2;

	I2C i2c;
	IMU imu1, imu2;
	i2c.init(I2C::Module::N1);
	bool ok1 = imu1.init(&i2c, MPU6050::Module::N1);
	bool ok2 = imu2.init(&i2c, MPU6050::Module::N2);
	
	GPIOE->ODR |= (1 << 10);

	while (true)
	{
		if (g_usb)
		{
			Quat q1 = imu1.read();
			Quat q2 = imu2.read();
			if ((GPIOA->IDR & 0x1))
			{
				qRef1 = q1.inverse();
				qRef2 = q2.inverse();
			}
			q1 = qRef1 * q1;
			q2 = qRef2 * q2;
			sprintf(buf, "{\"q1\":[%f,%f,%f,%f],\"q2\":[%f,%f,%f,%f]}\n",
                    q1[0], q1[1], q1[2], q1[3], q2[0], q2[1], q2[2], q2[3]);
			for (uint8_t i = 0; buf[i]; ++i)
				USB_Send_Data(buf[i]);
			USB_Send_Data(0);
		}
		/*else
		{
			button_scan();
		}*/
	}
	
	return 0;
}
