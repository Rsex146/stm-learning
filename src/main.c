
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_spi.h"
#include "stm32f30x_dma.h"


#define DMA_BUFF_SIZE 4

volatile uint8_t g_src[DMA_BUFF_SIZE] = { 1, 2, 3, 4 };
volatile uint8_t g_dst[DMA_BUFF_SIZE] = { 0 };
volatile uint8_t g_end[DMA_BUFF_SIZE] = { 0 };
volatile uint8_t g_dataReady = 0;

void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

void usart1()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef g;
	GPIO_StructInit(&g);

	g.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &g);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

	USART_InitTypeDef u;
	USART_StructInit(&u);
	u.USART_BaudRate = 9600;
	USART_Init(USART1, &u);

	USART_Cmd(USART1, ENABLE);
}

void usart2()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef g;
	GPIO_StructInit(&g);

	g.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &g);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

	USART_InitTypeDef u;
	USART_StructInit(&u);
	u.USART_BaudRate = 9600;
	USART_Init(USART2, &u);

	USART_Cmd(USART2, ENABLE);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);
}

void led()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef g;

	GPIO_StructInit(&g);
	g.GPIO_Pin = 0xFF00;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);
}

void buttonInterrupt()
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

void button()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef g;

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_IN;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &g);

	buttonInterrupt();
}

void EXTI0_IRQHandler()
{
	DMA_Cmd(DMA1_Channel1, ENABLE);

	EXTI_ClearFlag(EXTI_Line0);
}

void dma1()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef d;
	DMA_StructInit(&d);
	d.DMA_PeripheralBaseAddr = (uint32_t)&g_src;
	d.DMA_MemoryBaseAddr = (uint32_t)&g_dst;
	d.DMA_DIR = DMA_DIR_PeripheralSRC;
	d.DMA_BufferSize = 4;
	d.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	d.DMA_MemoryInc = DMA_MemoryInc_Enable;
	d.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	d.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	d.DMA_Mode = DMA_Mode_Normal;
	d.DMA_Priority = DMA_Priority_Medium;
	d.DMA_M2M = DMA_M2M_Enable;
	DMA_Init(DMA1_Channel1, &d);
	DMA_Cmd(DMA1_Channel1, DISABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void dma2()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	DMA_InitTypeDef d;
	DMA_StructInit(&d);
	d.DMA_PeripheralBaseAddr = (uint32_t)&g_dst;
	d.DMA_MemoryBaseAddr = (uint32_t)&g_end;
	d.DMA_DIR = DMA_DIR_PeripheralSRC;
	d.DMA_BufferSize = 4;
	d.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	d.DMA_MemoryInc = DMA_MemoryInc_Enable;
	d.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	d.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
	d.DMA_Mode = DMA_Mode_Normal;
	d.DMA_Priority = DMA_Priority_Medium;
	d.DMA_M2M = DMA_M2M_Enable;
	DMA_Init(DMA2_Channel1, &d);
	DMA_Cmd(DMA2_Channel1, DISABLE);
	DMA_ITConfig(DMA2_Channel1, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA2_Channel1_IRQn);
}

void usart1_send(uint8_t ch)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, ch);
}

void DMA1_Channel1_IRQHandler(void)
{
	g_dataReady = 1;

	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_Cmd(DMA1_Channel1, DISABLE);
}

void DMA2_Channel1_IRQHandler(void)
{
	GPIOE->ODR = (1 << (g_end[0] + 8)) | (1 << (g_end[1] + 8)) | (1 << (g_end[2] + 8)) | (1 << (g_end[3] + 8));

	DMA_ClearITPendingBit(DMA2_IT_TC1);
	DMA_Cmd(DMA2_Channel1, DISABLE);
}

void USART2_IRQHandler()
{
	static uint8_t bytesReceived = 0;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		++bytesReceived;
		if (bytesReceived == DMA_BUFF_SIZE)
		{
			bytesReceived = 0;
			DMA_Cmd(DMA2_Channel1, ENABLE);
		}
	}
}

int main()
{
	led();
	button();
	dma1();
	dma2();
	usart1();
	usart2();

	while (1)
	{
		if (g_dataReady)
		{
			g_dataReady = 0;
			for (uint8_t i = 0; i < DMA_BUFF_SIZE; ++i)
				usart1_send(g_dst[i]);
		}
	};

    return 0;
}
