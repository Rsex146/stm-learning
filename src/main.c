
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	for (i = 0; i < t; ++i){__NOP();};
}

int main()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
    GPIO_InitTypeDef g;

    g.GPIO_Pin = 0xFF00;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_Speed = GPIO_Speed_Level_1;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &g);

    uint16_t v = 0x1;

    while (1)
    {
    	//GPIO_SetBits(GPIOE, v);
    	GPIOE->ODR = v << 8;
    	v = (v << 1) % 0x00FF;

    	//GPIO_Set
    	//GPIOE->ODR = 512;
    	myDelay(3200000);
    	//GPIO_ResetBits(GPIOE, 0xFF00);
    	//GPIOE->ODR = 0;
    	//myDelay(3200000);
    }

    return 0;
}


