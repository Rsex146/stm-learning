
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"
#include "stm32f30x_tim.h"


#define LED(i) (1 << (8 + ((i) % 8)))
#define MIN(a, b) ((a) < (b) ? a : b)

static volatile uint16_t g_pinFrom = LED(0);
static volatile uint16_t g_pinTo = LED(1);
static volatile int32_t g_t = 0;

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

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_2);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_2);

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_AN;
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

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	ADC_StartConversion(ADC1);
}

void nvic()
{
	NVIC_InitTypeDef n;
	n.NVIC_IRQChannel = ADC1_2_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 1;
	n.NVIC_IRQChannelSubPriority = 3;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
}

void ADC1_2_IRQHandler(void)
{
	uint32_t l = ADC_GetConversionValue(ADC1);
	uint32_t ledIdx = l / 512;
	g_pinFrom = LED(ledIdx);
	g_pinTo = LED(ledIdx + 1);
	g_t = l % 512;
	GPIOE->ODR &= (g_pinFrom | g_pinTo);
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}

int main()
{
	gpio();
	nvic();
	adc();

	while(1)
	{
		myPWM(g_t, 512, g_pinTo, g_pinFrom);
	};

    return 0;
}
