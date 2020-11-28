
#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_misc.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_spi.h"


volatile uint8_t g_sendData;
volatile uint8_t g_receiveData[2];
volatile uint8_t g_tempByte;
volatile uint16_t g_xResult;
volatile uint8_t g_xSign;

double g_xPosition;


void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

	GPIO_InitTypeDef g;

	// Leds
	GPIO_StructInit(&g);
	g.GPIO_Pin = 0xFF00;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	// SS for gyro
	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_3;
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
}

void spi()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

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
	SPI_Cmd(SPI1, ENABLE);

	SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);
}

uint8_t sendByte(uint8_t data)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){};
	SPI_SendData8(SPI1, data);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){};
	return (uint8_t)SPI_ReceiveData8(SPI1);
}

void writeData(uint8_t address, uint8_t data)
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	sendByte(address);
	sendByte(data);
	GPIO_SetBits(GPIOE, GPIO_Pin_3);
}

void tim()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	TIM_TimeBaseInitTypeDef t;
	TIM_TimeBaseStructInit(&t);
	t.TIM_CounterMode = TIM_CounterMode_Up;
	t.TIM_Prescaler = 36000;
	t.TIM_Period = 40;
	TIM_TimeBaseInit(TIM7, &t);
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM7, ENABLE);
}

void nvic()
{
	NVIC_InitTypeDef n;
	n.NVIC_IRQChannel = TIM7_IRQn;
	n.NVIC_IRQChannelPreemptionPriority = 0;
	n.NVIC_IRQChannelSubPriority = 0;
	n.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&n);
}

void TIM7_IRQHandler()
{
	GPIO_ResetBits(GPIOE, GPIO_Pin_3); // Start talk
	sendByte(0xE8); // Start gyro conversion
	g_receiveData[0] = sendByte(0x00); // Get first byte of conversion
	g_receiveData[1] = sendByte(0x00); // Get second byte of conversion
	GPIO_SetBits(GPIOE, GPIO_Pin_3); // End talk

	g_xResult = (g_receiveData[0] | (g_receiveData[1] << 8)) - 10; // -10 is offset of gyro, get it calibrated!
	if ((g_xResult & 0x8000) == 0) // find out sign
		g_xSign = 0; // "+"
	else
	{
		g_xSign = 1; // "-"
		// Flip according to datasheet, this is reverse-code according to our gyro
		g_xResult &= 0x7FFF;
		g_xResult = 0x8000 - g_xResult;
	}
	if (g_xResult < 0x20) // Threshold to remove electrical/thermal bouncing of gyro, get it calibrated!
		g_xResult = 0;
	// 0.07 is degrees per second; 0.025 is sampling rate in seconds.
	// Normally it should be 0.02, but we are too slow in this interrupt handler, so we are using 0.025
	if (g_xSign == 0)
		g_xPosition += 0.07 * g_xResult * 0.025;
	else
		g_xPosition -= 0.07 * g_xResult * 0.025;

	GPIO_Write(GPIOE, 0x0000); // Switch off all LEDs

	// Map angle to LEDs
	if ((g_xPosition > -105) && (g_xPosition < -75))
		GPIO_SetBits(GPIOE, GPIO_Pin_8);
	if ((g_xPosition > -75) && (g_xPosition < -45))
		GPIO_SetBits(GPIOE, GPIO_Pin_9);
	if ((g_xPosition > -45) && (g_xPosition < -15))
		GPIO_SetBits(GPIOE, GPIO_Pin_10);
	if ((g_xPosition > -15) && (g_xPosition < 15))
		GPIO_SetBits(GPIOE, GPIO_Pin_11);
	if ((g_xPosition > 15) && (g_xPosition < 45))
		GPIO_SetBits(GPIOE, GPIO_Pin_12);
	if ((g_xPosition > 45) && (g_xPosition < 75))
		GPIO_SetBits(GPIOE, GPIO_Pin_13);
	if ((g_xPosition > 75) && (g_xPosition < 105))
		GPIO_SetBits(GPIOE, GPIO_Pin_14);

	TIM_ClearITPendingBit(TIM7, TIM_IT_Update); // Clear interrupt flag.
}

int main()
{
	gpio();
	spi();
	nvic();

	g_xPosition = 0;

	writeData(0x20, 0x0A);
	writeData(0x23, 0x30);

	tim();

	while (1)
	{
		__NOP();
	};

    return 0;
}
