
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


uint16_t m1[2] = {1000, GPIO_Pin_13};
uint16_t m2[2] = {100, GPIO_Pin_14};

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
	GPIO_InitTypeDef g;

	g.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	g.GPIO_Mode = GPIO_Mode_OUT;
	g.GPIO_Speed = GPIO_Speed_Level_1;
	g.GPIO_OType = GPIO_OType_PP;
	g.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &g);
}

void taskLED(void *param)
{
	while (1)
	{
		GPIO_SetBits(GPIOE, ((uint16_t *)param)[1]);
		vTaskDelay(((uint16_t *)param)[0]);
		GPIO_ResetBits(GPIOE, ((uint16_t *)param)[1]);
		vTaskDelay(((uint16_t *)param)[0]);
	}
}

int main()
{
	gpio();

	xTaskCreate(taskLED, (char *)"LED1", configMINIMAL_STACK_SIZE, m1, 2, (xTaskHandle *)NULL);
	xTaskCreate(taskLED, (char *)"LED2", configMINIMAL_STACK_SIZE, m2, 2, (xTaskHandle *)NULL);
	vTaskStartScheduler();

	while (1)
	{
		__NOP();
	}

	return 0;
}

