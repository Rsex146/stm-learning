
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

    g.GPIO_Pin = GPIO_Pin_9;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_Speed = GPIO_Speed_Level_1;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &g);

    while (1)
    {
    	GPIO_SetBits(GPIOE, GPIO_Pin_9);
    	myDelay(3200000);
    	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
    	myDelay(3200000);
    }

    return 0;
}


