
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_spi.h"


void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

void SPI2_IRQHandler()
{
	GPIOE->ODR = 1 << (SPI_ReceiveData8(SPI2) + 8);
}

void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef g;

	// Button
	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_IN;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &g);

	// Leds
	GPIO_StructInit(&g);
	g.GPIO_Pin = 0xFF00;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	// SPI1
	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &g);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);

	// SPI2
	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	g.GPIO_Mode = GPIO_Mode_AF;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &g);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_5);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_5);
}

void spi()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	SPI_InitTypeDef s;

	SPI_StructInit(&s);
	s.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	s.SPI_DataSize = SPI_DataSize_8b;
	s.SPI_CPOL = SPI_CPOL_High;
	s.SPI_CPHA = SPI_CPHA_2Edge;
	s.SPI_NSS = SPI_NSS_Soft;
	s.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	s.SPI_FirstBit = SPI_FirstBit_MSB;
	s.SPI_CRCPolynomial = 7;
	s.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPI1, &s);
	SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);
	SPI_Cmd(SPI1, ENABLE);

	SPI_StructInit(&s);
	s.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	s.SPI_DataSize = SPI_DataSize_8b;
	s.SPI_CPOL = SPI_CPOL_High;
	s.SPI_CPHA = SPI_CPHA_2Edge;
	s.SPI_NSS = SPI_NSS_Soft;
	s.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	s.SPI_FirstBit = SPI_FirstBit_MSB;
	s.SPI_CRCPolynomial = 7;
	s.SPI_Mode = SPI_Mode_Slave;
	SPI_Init(SPI2, &s);
	SPI_Cmd(SPI2, ENABLE);

	SPI_RxFIFOThresholdConfig(SPI2, SPI_RxFIFOThreshold_QF);

	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
	NVIC_EnableIRQ(SPI2_IRQn);
}

int main()
{
	gpio();
	spi();

	uint8_t f = 0, i = 0;

	while (1)
	{
		if (((GPIOA->IDR & (1 << 0)) == 1) && (f == 0))
		{
			myDelay(50000);
			if (((GPIOA->IDR & (1 << 0)) == 1) && (f == 0))
			{
				SPI_SendData8(SPI1, (i++) % 8);
				f = 1;
			}
		}
		if (((GPIOA->IDR & (1 << 0)) == 0) && (f == 1))
		{
			f = 0;
		}
	};

    return 0;
}
