
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_exti.h"
#include "stm32f30x_syscfg.h"


#define LED(i) (1 << (8 + ((i) % 8)))
#define MIN(a, b) ((a) < (b) ? a : b)

static volatile uint32_t g_adcVal = 0;
static volatile uint8_t g_updateMax = 0;


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

void myPWM(int32_t tau, int32_t T, uint16_t pinAverse, uint16_t pinReverse)
{
	if (tau >= T)
	{
		GPIO_SetBits(GPIOE, pinAverse);
		GPIO_ResetBits(GPIOE, pinReverse);
	}
	else if (tau > 0)
	{
		GPIO_SetBits(GPIOE, pinAverse);
		GPIO_ResetBits(GPIOE, pinReverse);
		myDelay((uint32_t)tau);
		GPIO_ResetBits(GPIOE, pinAverse);
		GPIO_SetBits(GPIOE, pinReverse);
		myDelay((uint32_t)(T - tau));
	}
	else
	{
		GPIO_ResetBits(GPIOE, pinAverse);
		GPIO_SetBits(GPIOE, pinReverse);
	}
}

void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef g;

	GPIO_StructInit(&g);
	g.GPIO_Pin = 0xFF00;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_1;
	g.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &g);

	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_IN;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &g);
}

void adc()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div10);

	ADC_InitTypeDef a;
	ADC_StructInit(&a);

	a.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	a.ADC_Resolution = ADC_Resolution_12b;
	a.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	a.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	a.ADC_DataAlign = ADC_DataAlign_Right;
	a.ADC_OverrunMode = DISABLE;
	a.ADC_AutoInjMode = DISABLE;
	a.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &a);

	ADC_Cmd(ADC1, ENABLE);

	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_7Cycles5);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	ADC_StartConversion(ADC1);
}

void nvic()
{
	NVIC_InitTypeDef n;
	n.NVIC_IRQChannel = ADC1_2_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 0;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);

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

void ADC1_2_IRQHandler(void)
{
	g_adcVal = ADC_GetConversionValue(ADC1);
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}

void EXTI0_IRQHandler()
{
	g_updateMax = 1;
	EXTI_ClearFlag(EXTI_Line0);
}

int main()
{
	gpio();
	nvic();
	adc();

	uint16_t pinFrom;
	uint16_t pinTo;
	int32_t t;
	uint32_t ledIdx;
	uint32_t val;
	uint32_t maxVal = 4096;

	while (1)
	{
		if (g_updateMax)
		{
			maxVal = g_adcVal;
			g_updateMax = 0;
		}
		val = g_adcVal;
		if (val > maxVal)
			val = 4096;
		ledIdx = val / 512;
		pinFrom = LED(ledIdx);
		pinTo = LED(ledIdx + 1);
		t = val % 512;
		GPIOE->ODR &= (pinFrom | pinTo);
		myPWM(t, 512, pinTo, pinFrom);
	};

    return 0;
}
