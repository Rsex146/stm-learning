
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_tim.h"


void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	GPIO_InitTypeDef g;

	g.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_2);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_2);
}

void tim()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitTypeDef t;
	TIM_TimeBaseStructInit(&t);
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_Prescaler = 3600;
	t.TIM_Period = 100;
	TIM_TimeBaseInit(TIM1, &t);
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

void pwm()
{
	TIM_OCInitTypeDef p;
	p.TIM_Pulse = 0;
	p.TIM_OCMode = TIM_OCMode_PWM1;
	p.TIM_OutputState = TIM_OutputState_Enable;
	p.TIM_OutputNState = TIM_OutputNState_Enable;
	p.TIM_OCPolarity = TIM_OCPolarity_High;
	p.TIM_OCNPolarity = TIM_OCNPolarity_High;
	p.TIM_OCIdleState = TIM_OCIdleState_Set;
	p.TIM_OCNIdleState = TIM_OCNIdleState_Set;
	TIM_OC1Init(TIM1, &p);
}

void nvic()
{
	NVIC_InitTypeDef n;
	n.NVIC_IRQChannel = TIM1_UP_TIM16_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 0;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
}

void TIM1_UP_TIM16_IRQHandler()
{
	static uint16_t pulse = 0;
	static uint8_t ph = 1;
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	pulse += ph ? 1 : -1;
	if (pulse == 100 || pulse == 0)
		ph = !ph;
	TIM1->CCR1 = pulse;
}

int main()
{
	gpio();
	nvic();
	tim();
	pwm();

	while(1)
	{
		__NOP();
	};

    return 0;
}


