
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_syscfg.h"


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

	g.GPIO_Pin = GPIO_Pin_9;
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

void ex()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	EXTI_InitTypeDef e;
	e.EXTI_Line = EXTI_Line0;
	e.EXTI_Mode = EXTI_Mode_Interrupt;
	e.EXTI_Trigger = EXTI_Trigger_Rising;
	e.EXTI_LineCmd = ENABLE;
	EXTI_Init(&e);

	NVIC_InitTypeDef nv;
	nv.NVIC_IRQChannel = EXTI0_IRQn;
	nv.NVIC_IRQChannelPreemptionPriority = 0;
	nv.NVIC_IRQChannelSubPriority = 0;
	nv.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nv);
}

int on = 0;

void EXTI0_IRQHandler()
{
	if (on)
		GPIO_SetBits(GPIOE, GPIO_Pin_9);
	else
		GPIO_ResetBits(GPIOE, GPIO_Pin_9);
	on = !on;
	EXTI_ClearFlag(EXTI_Line0);
}

int main()
{
	gpio();
	ex();

    while (1) { __NOP(); };

    return 0;
}


