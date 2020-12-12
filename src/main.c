
#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portmacro.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "croutine.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"


xSemaphoreHandle btnON;
xSemaphoreHandle btnOFF;

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

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_13;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);

	GPIO_StructInit(&g);
	g.GPIO_Pin = GPIO_Pin_0;
	g.GPIO_Mode = GPIO_Mode_IN;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &g);
}

void taskLED_ON()
{
	while (1)
	{
		xSemaphoreTake(btnON, portMAX_DELAY);
		GPIO_SetBits(GPIOE, GPIO_Pin_13);
	}
}

void taskLED_OFF()
{
	while (1)
	{
		xSemaphoreTake(btnOFF, portMAX_DELAY);
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

	vSemaphoreCreateBinary(btnON);
	vSemaphoreCreateBinary(btnOFF);

	xTaskCreate(taskLED_ON, (char *)"LED", configMINIMAL_STACK_SIZE, NULL, 2, (xTaskHandle *)NULL);
	xTaskCreate(taskLED_OFF, (char *)"LED", configMINIMAL_STACK_SIZE, NULL, 2, (xTaskHandle *)NULL);
	xTaskCreate(taskButtonScan, (char *)"BUTTONSCAN", configMINIMAL_STACK_SIZE, NULL, 2, (xTaskHandle *)NULL);
	vTaskStartScheduler();

	while (1)
	{
		__NOP();
	}

	return 0;
}

