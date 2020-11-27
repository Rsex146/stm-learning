
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_dma.h"
#include "stm32f30x_adc.h"


void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef g;

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &g);

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_2);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_2);
}

void tim()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	TIM_TimeBaseInitTypeDef t;
	TIM_TimeBaseStructInit(&t);
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_Prescaler = 36;
	t.TIM_Period = 4095;
	TIM_TimeBaseInit(TIM1, &t);
	TIM_Cmd(TIM1, ENABLE);
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

void dma()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef d;
	DMA_StructInit(&d);
	d.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	d.DMA_MemoryBaseAddr = (uint32_t)&(TIM1->CCR1);
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

	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) != RESET);

	ADC_InitTypeDef a;
	ADC_StructInit(&a);

	a.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	a.ADC_Resolution = ADC_Resolution_12b;
	a.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
	a.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	a.ADC_DataAlign = ADC_DataAlign_Right;
	a.ADC_OverrunMode = ADC_OverrunMode_Disable;
	a.ADC_AutoInjMode = ADC_AutoInjec_Disable;
	a.ADC_NbrOfRegChannel = 1;
	ADC_Init(ADC1, &a);

	ADC_Cmd(ADC1, ENABLE);
}

void startADC()
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
	ADC_StartConversion(ADC1);
}

int main()
{
	adc();
	gpio();
	tim();
	pwm();
	dma();

	startADC();

	while (1)
	{
		__NOP();
	};

    return 0;
}
