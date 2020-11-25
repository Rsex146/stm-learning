
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_adc.h"


volatile uint16_t g_result = 0;


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
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

void dma()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef d;
	DMA_StructInit(&d);
	d.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	d.DMA_MemoryBaseAddr = (uint32_t)&g_result;
	d.DMA_DIR = DMA_DIR_PeripheralSRC;
	d.DMA_BufferSize = DMA_PeripheralDataSize_HalfWord;
	d.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	d.DMA_MemoryInc = DMA_MemoryInc_Disable;
	d.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	d.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	d.DMA_Mode = DMA_Mode_Circular;
	d.DMA_Priority = DMA_Priority_High;
	d.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &d);
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void adc()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div10);

	ADC_CommonInitTypeDef ac;
	ADC_CommonStructInit(&ac);

	ac.ADC_Mode = ADC_Mode_Independent;
	ac.ADC_Clock = ADC_Clock_AsynClkMode;
	ac.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ac.ADC_DMAMode = ADC_DMAMode_Circular;
	ac.ADC_TwoSamplingDelay = 0;
	ADC_CommonInit(ADC1, &ac);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);

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

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	ADC_StartConversion(ADC1);
}

void pwm(uint32_t t)
{
	GPIOE->ODR |= (1 << 9);
	myDelay(t);
	GPIOE->ODR &= ~(1 << 9);
	myDelay(4095 - t);
}

int main()
{
	adc();
	gpio();
	dma();

	while (1)
	{
		pwm(g_result);
	};

    return 0;
}
