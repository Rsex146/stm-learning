
// #include <hw_config.h>
// #include <usb_lib.h>
// #include <usb_desc.h>
// #include <usb_pwr.h>


#include "i2c_lib.h"


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

int main()
{
	led();
	GPIOE->ODR = (1 << 9);

	uint8_t who;
	I2C i2c;
	i2c.init(I2C::Module::N1);
	i2c.read(0x68, 0x75, &who, 1);
	
	while (true)
	{
		__NOP();
	}
	return 0;
}
