
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i){__NOP();};
}

int main()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef g;

    g.GPIO_Pin = GPIO_Pin_8;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_Speed = GPIO_Speed_Level_1;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &g);

    g.GPIO_Pin = GPIO_Pin_0;
    g.GPIO_Mode = GPIO_Mode_IN;
    g.GPIO_Speed = GPIO_Speed_Level_1;
    g.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &g);

    uint8_t pushed = 0;

    while (1)
    {
    	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    	{
    		if (!pushed)
    		{
				myDelay(50000);
				if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
				{
					pushed = 1;
					GPIOE->ODR ^= GPIO_Pin_8;
				}
    		}
    	}
    	else
    	{
    		pushed = 0;
    	}
    }

    return 0;
}
