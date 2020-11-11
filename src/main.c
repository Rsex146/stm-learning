
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include <math.h>


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i){__NOP();};
}

void PWM(uint32_t tau, uint32_t T, uint16_t pin)
{
	GPIO_SetBits(GPIOE, pin);
	myDelay(tau);
	GPIO_ResetBits(GPIOE, pin);
	myDelay(T - tau);
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

    float a = 0.0f;

    while (1)
    {
    	for (uint8_t i = 0; i < 8; ++i)
    		PWM((sinf(a + 2 * 3.14f * i / 8) + 1) * 500, 1000, 0x0100 << i);
    	a += 0.01f;
    	if (a > 6.28f)
    		a = 0.0f;
    }

    return 0;
}


