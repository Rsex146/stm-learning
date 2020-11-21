
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_adc.h"


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

uint32_t readADC(uint8_t channel)
{
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_7Cycles5);
	ADC_StartConversion(ADC1);
	return ADC_GetConversionValue(ADC1);
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

	a.ADC_ContinuousConvMode = DISABLE;
	a.ADC_Resolution = ADC_Resolution_12b;
	a.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	a.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	a.ADC_DataAlign = ADC_DataAlign_Right;
	a.ADC_OverrunMode = DISABLE;
	a.ADC_AutoInjMode = DISABLE;
	a.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &a);
	ADC_Cmd(ADC1, ENABLE);
}

int main()
{
	adc();
	gpio();

	while(1)
	{
		uint32_t result = readADC(ADC_Channel_1);
		myPWM(result, 1 << 12, 1 << 8);

	};

    return 0;
}


