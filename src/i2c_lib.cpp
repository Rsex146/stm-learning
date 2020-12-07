
#include "i2c_lib.h"


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
    i2c.I2C_Timing = 0xC062121F;
    I2C_Init(m_i2c, &i2c);
    
    I2C_Cmd(m_i2c, ENABLE);
}

void I2C::read(uint32_t address, uint8_t regName, uint8_t *buffer, uint8_t len)
{
    // Convert address to 7bit format.
    address = (address & 0x7F) << 1; 

    // Specify register to read.
    while (I2C_GetFlagStatus(m_i2c, I2C_FLAG_BUSY) != RESET);
    I2C_TransferHandling(m_i2c, address, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_TXIS) == RESET);
    I2C_SendData(m_i2c, regName);
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_TC) == RESET);

    // Read data.
    I2C_TransferHandling(m_i2c, address, len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
    for (uint8_t i = 0; i < len; ++i)
    {
        while (I2C_GetFlagStatus(m_i2c, I2C_ISR_RXNE) == RESET);
        buffer[i] = I2C_ReceiveData(m_i2c);
    }

    // Stop communication.
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_STOPF) == RESET);
    I2C_ClearFlag(m_i2c, I2C_ICR_STOPCF);
}
    
void I2C::write(uint32_t address, uint8_t regName, const uint8_t *buffer, uint8_t len)
{
    // Convert address to 7bit format.
    address = (address & 0x7F) << 1; 

    // Specify register to write.
    while (I2C_GetFlagStatus(m_i2c, I2C_FLAG_BUSY) != RESET);
    I2C_TransferHandling(m_i2c, address, len + 1, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_TXIS) == RESET);
    I2C_SendData(m_i2c, regName);
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_TC) == RESET);

    // Write data.
    for (uint8_t i = 0; i < len; ++i)
    {
        I2C_SendData(m_i2c, (uint8_t)buffer[i]);
        while (I2C_GetFlagStatus(m_i2c, I2C_ISR_TC) == RESET);
    }

    // Stop communication.
    while (I2C_GetFlagStatus(m_i2c, I2C_ISR_STOPF) == RESET);
    I2C_ClearFlag(m_i2c, I2C_ICR_STOPCF);
}
