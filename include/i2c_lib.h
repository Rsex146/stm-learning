
#ifndef __I2C_LIB_H__
#define __I2C_LIB_H__

#include "stm32f30x_gpio.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_i2c.h"


class I2C
{
public:
    enum Module
    {
        N1,
        N2
    };

private:
    enum Direction
    {
        IN,
        OUT
    };

    I2C_TypeDef *m_i2c;

    void beginTransaction(Direction dir, uint32_t address, uint8_t len);
    void endTransaction();

public:
    I2C();

    void init(Module module);
    bool read(uint32_t address, uint8_t regName, uint8_t *buffer, uint8_t len);
    bool write(uint32_t address, uint8_t regName, const uint8_t *buffer, uint8_t len);
};

#endif //__I2C_LIB_H__
