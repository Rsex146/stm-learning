
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "croutine.h"
#include "task.h"
#include "queue.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"


xQueueHandle g_led3;

void vApplicationIdleHook ( void ){}
void vApplicationMallocFailedHook ( void ){for ( ;; );}
void vApplicationStackOverflowHook ( xTaskHandle pxTask, char *pcTaskName ){
( void ) pcTaskName;
( void ) pxTask;
for ( ;; );}
void vApplicationTickHook ( void ){}

void gpio()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef g;

	g.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_IN;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &g);
}

void taskLED()
{
	uint8_t btn;

	while (1)
	{
		xQueueReceive(g_led3, &btn, 0);
		if (btn == 1)
			GPIO_SetBits(GPIOE, GPIO_Pin_13);
		else if (btn == 0)
			GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	}
}

void taskButtonScan()
{
	uint8_t a = 1;
	uint8_t b = 0;
	uint8_t state = 0;
	while (1)
	{
		state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		if (state == 1)
			xQueueSend(g_led3, &a, 0);
		else if (state == 0)
			xQueueSend(g_led3, &b, 0);
	}
}

int main()
{
	gpio();

	g_led3 = xQueueCreate(1, sizeof(uint8_t));

	xTaskCreate(taskLED, (char *)"LED", configMINIMAL_STACK_SIZE, NULL, 2, (xTaskHandle *)NULL);
	xTaskCreate(taskButtonScan, (char *)"BUTTONSCAN", configMINIMAL_STACK_SIZE, NULL, 2, (xTaskHandle *)NULL);
	vTaskStartScheduler();

	while (1)
	{
		__NOP();
	}

	return 0;
}

