
#include "timer_lib.h"


Timer g_timer;

extern "C" void SysTick_Handler()
{
    g_timer.tick();
}

void Timer::init()
{
    RCC_ClocksTypeDef c;
    RCC_GetClocksFreq(&c);
    SysTick_Config(c.HCLK_Frequency / 1000);
}
