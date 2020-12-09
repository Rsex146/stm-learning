
#include <stdio.h>

#include <hw_config.h>
#include <usb_lib.h>
#include <usb_desc.h>
#include <usb_pwr.h>

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
			g_usb = true;
			f = 1;
		}
	}
	if (!(GPIOA->IDR & 0x1) && (f == 1))
	{
		f = 0;
	}
}

int main()
{
	led();
	button();

	GPIOE->ODR = (1 << 9);

	/*uint8_t who;
	I2C i2c;
	i2c.init(I2C::Module::N1);
	i2c.read(0x68, 0x75, &who, 1);*/

	char buf[255];

	IMU imu;
	bool ok = imu.init();
	
	while (true)
	{
		if (g_usb)
		{
			Quat q = imu.read();
			sprintf(buf, "{\"q\":[%f,%f,%f,%f]}\n", q[0], q[1], q[2], q[3]);
			for (uint8_t i = 0; buf[i]; ++i)
				USB_Send_Data(buf[i]);
			USB_Send_Data(0);
		}
		else
		{
			button_scan();
		}
	}
	
	return 0;
}