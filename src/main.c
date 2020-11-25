
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_dma.h"


#define DMA_BUFF_SIZE 4

volatile uint16_t dst[DMA_BUFF_SIZE] = {0};
volatile uint16_t src[DMA_BUFF_SIZE] = { 0x0100, 0x0200, 0x0400, 0x0800 };


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

void DMA1_Channel1_IRQHandler(void)
{
	GPIOE->ODR = dst[0] | dst[1] | dst[2] | dst[3];
	DMA_ClearITPendingBit(DMA1_IT_TC1);
	DMA_Cmd(DMA1_Channel1, DISABLE);
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

void dma()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef d;
	DMA_StructInit(&d);
	d.DMA_PeripheralBaseAddr = (uint32_t)&src;
	d.DMA_MemoryBaseAddr = (uint32_t)&dst;
	d.DMA_DIR = DMA_DIR_PeripheralSRC;
	d.DMA_BufferSize = 10;
	d.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
	d.DMA_MemoryInc = DMA_MemoryInc_Enable;
	d.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	d.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	d.DMA_Mode = DMA_Mode_Normal;
	d.DMA_Priority = DMA_Priority_Medium;
	d.DMA_M2M = DMA_M2M_Enable;
	DMA_Init(DMA1_Channel1, &d);
	DMA_Cmd(DMA1_Channel1, DISABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

int main()
{
	gpio();
	dma();

	uint8_t f = 0;

	while (1)
	{
		if (((GPIOA->IDR & (1 << 0)) == 1) && (f == 0))
		{
			myDelay(50000);

			if (((GPIOA->IDR & (1 << 0)) == 1) && (f == 0))
			{
				DMA_Cmd(DMA1_Channel1, ENABLE);
				f = 1;
			}
		}
		if (((GPIOA->IDR & (1 << 0)) == 0) && (f == 0))
		{
			f = 0;
		}
	};

    return 0;
}
