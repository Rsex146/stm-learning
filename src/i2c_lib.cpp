
#include "i2c_lib.h"


#define I2C_OFFSET_TIMINGR_SCLL   0
#define I2C_OFFSET_TIMINGR_SCLH   8
#define I2C_OFFSET_TIMINGR_SDADEL 16
#define I2C_OFFSET_TIMINGR_SCLDEL 20
#define I2C_OFFSET_TIMINGR_PRESC  28
#define I2C_OFFSET_CR2_NBYTES     16

I2C::I2C()
{
    m_i2c = nullptr;
}

void I2C::init(Module module)
{
    uint32_t periphI2C = 0;
    uint32_t sysClock = 0;
    uint32_t gpioPins = 0;
    uint16_t pinSourceSCL = 0;
    uint16_t pinSourceSDA = 0;

    switch (module)
    {
        case Module::N1:
            m_i2c = I2C1;
            periphI2C = RCC_APB1Periph_I2C1;
            sysClock = RCC_I2C1CLK_SYSCLK;
            gpioPins = GPIO_Pin_6 | GPIO_Pin_7;
            pinSourceSCL = GPIO_PinSource6;
            pinSourceSDA = GPIO_PinSource7;
            break;

        case Module::N2:
            m_i2c = I2C2;
            periphI2C = RCC_APB1Periph_I2C2;
            sysClock = RCC_I2C2CLK_SYSCLK;
            gpioPins = GPIO_Pin_9 | GPIO_Pin_10;
            pinSourceSCL = GPIO_PinSource9;
            pinSourceSDA = GPIO_PinSource10;
            break;
    }

    RCC_I2CCLKConfig(sysClock);
    RCC_APB1PeriphClockCmd(periphI2C, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = gpioPins;
    gpio.GPIO_Mode = GPIO_Mode_AF; 
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);
    GPIO_PinAFConfig(GPIOB, pinSourceSCL, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, pinSourceSDA, GPIO_AF_4);

    I2C_InitTypeDef i2c;
    I2C_StructInit(&i2c);
    i2c.I2C_Mode = I2C_Mode_I2C;  
    i2c.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    i2c.I2C_DigitalFilter = 0x00;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_Timing = 0x00310309; // I2C speed 400 KHz, whereby HSI = 8 MHz
    I2C_Init(m_i2c, &i2c);
    
    I2C_Cmd(m_i2c, ENABLE);
}

void I2C::beginTransaction(Direction dir, uint32_t address, uint8_t len)
{
    switch (dir)
    {
        case Direction::IN:
            m_i2c->CR2 |= I2C_CR2_RD_WRN;
            break;

        case Direction::OUT:
            m_i2c->CR2 &= ~I2C_CR2_RD_WRN;
            break;
    }
    m_i2c->CR2 &= ~I2C_CR2_NBYTES; // Clear data size field.
    m_i2c->CR2 |= len << I2C_OFFSET_CR2_NBYTES; // Set desired data size.
    m_i2c->CR2 &= ~I2C_CR2_SADD; // Clear slave address.
    m_i2c->CR2 |= address; // Set desired slave address.
    m_i2c->CR2 |= I2C_CR2_START; // Generate start on the bus.
    while ((m_i2c->ISR & I2C_ISR_BUSY) == 0); // Wait for the start being generated.
}

void I2C::endTransaction()
{
    m_i2c->CR2 |= I2C_CR2_STOP; // Generate stop on the bus.
    while (m_i2c->ISR & I2C_ISR_BUSY); // Wait for the stop being generated.
    m_i2c->ICR |= I2C_ICR_STOPCF; // Clear STOP flag.
    m_i2c->ICR |= I2C_ICR_NACKCF; // Clear NACK flag.
    // Clear error flags if any.
    if (m_i2c->ISR & (I2C_ISR_ARLO | I2C_ISR_BERR))
    {
        m_i2c->ICR |= I2C_ICR_ARLOCF;
        m_i2c->ICR |= I2C_ICR_BERRCF;
    }
}

bool I2C::read(uint32_t address, uint8_t regName, uint8_t *buffer, uint8_t len)
{
    // Convert address to 7bit format.
    address = (address & 0x7F) << 1; 
    
    beginTransaction(Direction::OUT, address, 1);
    while ((((m_i2c->ISR & I2C_ISR_TC) == 0) && ((m_i2c->ISR & I2C_ISR_NACKF) == 0)) && (m_i2c->ISR & I2C_ISR_BUSY))
    {
        if (m_i2c->ISR & I2C_ISR_TXIS)
            m_i2c->TXDR = regName;
    }

    uint8_t bytesRead = 0;
    beginTransaction(Direction::IN, address, len);
    while ((((m_i2c->ISR & I2C_ISR_TC) == 0) && ((m_i2c->ISR & I2C_ISR_NACKF) == 0)) && (m_i2c->ISR & I2C_ISR_BUSY))
    {
        if (m_i2c->ISR & I2C_ISR_RXNE)
            buffer[bytesRead++] = m_i2c->RXDR;
    }
    
    endTransaction();
    return bytesRead == len;
}
    
bool I2C::write(uint32_t address, uint8_t regName, const uint8_t *buffer, uint8_t len)
{
    // Convert address to 7bit format.
    address = (address & 0x7F) << 1; 

    beginTransaction(Direction::OUT, address, len + 1);
    while ((((m_i2c->ISR & I2C_ISR_TXIS) == 0) && ((m_i2c->ISR & I2C_ISR_NACKF) == 0)) && (m_i2c->ISR & I2C_ISR_BUSY));
    if (m_i2c->ISR & I2C_ISR_TXIS)
        m_i2c->TXDR = regName;
    
    uint8_t bytesWritten = 0;
    while ((((m_i2c->ISR & I2C_ISR_TC) == 0) && ((m_i2c->ISR & I2C_ISR_NACKF) == 0)) && (m_i2c->ISR & I2C_ISR_BUSY))
    {
        if (m_i2c->ISR & I2C_ISR_TXIS)
            m_i2c->TXDR = buffer[bytesWritten++];
    }
    
    endTransaction();
    return bytesWritten == len;
}
