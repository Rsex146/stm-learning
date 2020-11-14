
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_tim.h"


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

void gpio()
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
}

void nvic()
{
	NVIC_InitTypeDef nv;
	nv.NVIC_IRQChannel = TIM7_IRQn;
	nv.NVIC_IRQChannelPreemptionPriority = 0;
	nv.NVIC_IRQChannelSubPriority = 0;
	nv.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nv);
}

void tim()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	TIM_TimeBaseInitTypeDef t;
	TIM_TimeBaseStructInit(&t);
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_Prescaler = 36000;
	t.TIM_Period = 1000;
	TIM_TimeBaseInit(TIM7, &t);

	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM7, ENABLE);
}

void TIM7_IRQHandler()
{
	GPIOE->ODR ^= (1 << 8);
	TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
}

int main()
{
	gpio();
	nvic();
	tim();

	//nt32_t t = 0;
	//GPIOE->ODR |= (1 << 8);
	while(1)
	{
		/*t++;
		if (t % 50000)
			GPIOE->ODR ^= (1 << 8);*/
		__NOP();
	};

    return 0;
}


