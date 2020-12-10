
#ifndef __TIMER_LIB_H__
#define __TIMER_LIB_H__

#include "stm32f30x_rcc.h"
#include "stm32f30x_syscfg.h"


class Timer
{
    volatile uint32_t m_millis;

public:
    Timer()
    {
        m_millis = 0;
    };

    void init();

    void tick()
    {
        ++m_millis;
    };

    void reset()
    {
        m_millis = 0;
    };
    
    uint32_t millis() const
    {
        return m_millis;
    };
    
    void wait(uint32_t timeout)
    {
        uint32_t w = m_millis + timeout;
        while (m_millis < w)
        {
            __NOP();
        }
    };
};

extern Timer g_timer;

#endif // __TIMER_LIB_H__
