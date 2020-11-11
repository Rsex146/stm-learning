
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include <math.h>

#define MAX_LIGHTNESS 1000
#define DROP_INTERVAL 200


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i){__NOP();};
}

void myPWM(int32_t tau, int32_t T, uint16_t pin)
{
	if (tau >= T)
	{
		GPIO_SetBits(GPIOE, pin);
	}
	else if (tau > 0)
	{
		GPIO_SetBits(GPIOE, pin);
		myDelay((uint32_t)tau);
		GPIO_ResetBits(GPIOE, pin);
		myDelay((uint32_t)(T - tau));
	}
	else
	{
		GPIO_ResetBits(GPIOE, pin);
	}
}

int main()
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitTypeDef g;

    g.GPIO_Pin = 0xFF00;
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

    float lightness = 0.0f;
    int drop = 0;

    while (1)
    {
    	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 1)
    	{
    		++drop;
    		lightness += 1.0f;
    		if ((uint32_t)lightness > MAX_LIGHTNESS * 8)
    		{
    			lightness = (float)(MAX_LIGHTNESS * 8);
    			myDelay(50000);
    		}
    	}
    	else
    	{
			if (drop > 0 && drop < DROP_INTERVAL)
				lightness = 0.0f;
			drop = 0;
    	}
    	for (uint8_t i = 0; i < 8; ++i)
    		myPWM(((uint32_t)lightness - MAX_LIGHTNESS * i), MAX_LIGHTNESS, 1 << (i + 8));
    }

    return 0;
}


