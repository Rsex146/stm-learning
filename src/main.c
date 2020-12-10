
#include <hw_config.h>
#include <usb_lib.h>
#include <usb_desc.h>
#include <usb_pwr.h>


int32_t g_tau[8] = { 0 };

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
}

void myDelay(uint32_t t)
{
	uint32_t i = 0;
	t *= 7.2;
	for (i = 0; i < t; ++i) { __NOP(); };
}

const uint8_t MPWM_CHANNEL_CNT = 8;
const uint16_t MPWM_PINS[8] =
{
	GPIO_Pin_8,
	GPIO_Pin_9,
	GPIO_Pin_10,
	GPIO_Pin_11,
	GPIO_Pin_12,
	GPIO_Pin_13,
	GPIO_Pin_14,
	GPIO_Pin_15,
};
#define MPWM_PORT GPIOE

void myPWM(int32_t *tau, uint32_t T)
{
	int32_t curTau = 0;
	int32_t lastTau = 0;
	do
	{
		int32_t nextTau = T;
		for (uint8_t i = 0; i < MPWM_CHANNEL_CNT; ++i)
		{
			if (tau[i] > curTau)
			{
				GPIO_SetBits(MPWM_PORT, MPWM_PINS[i]);
				if (tau[i] < nextTau)
					nextTau = tau[i];
			}
			else
			{
				GPIO_ResetBits(MPWM_PORT, MPWM_PINS[i]);
			}
			if (tau[i] > lastTau)
			{
				lastTau = tau[i];
				if (lastTau > (int32_t)T)
					lastTau = (int32_t)T;
			}
		}
		myDelay((uint32_t)(nextTau - curTau));
		curTau = nextTau;
	}
	while (curTau < lastTau);
	for (uint8_t i = 0; i < MPWM_CHANNEL_CNT; ++i)
	{
		if (tau[i] < (int32_t)T)
			GPIO_ResetBits(MPWM_PORT, MPWM_PINS[i]);
	}
	myDelay((uint32_t)(T - lastTau));
}

int main()
{
	led();
	button();

	if (GPIOA->IDR & 0x1)
	{
		Set_System();
		Set_USBClock();
		USB_Interrupts_Config();
		USB_Init();
	}

	while (1)
	{
		myPWM(g_tau, 255);
	}

	return 0;
}
