
#include "I2C_Software_Master.h"

#define SCL_H         GPIOB->BSRRL = GPIO_Pin_6
#define SCL_L         GPIOB->BSRRH = GPIO_Pin_6

#define SDA_H         GPIOB->BSRRL = GPIO_Pin_9
#define SDA_L         GPIOB->BSRRH = GPIO_Pin_9 

#define SCL_read      GPIOB->IDR & GPIO_Pin_6
#define SDA_read      GPIOB->IDR & GPIO_Pin_9

#define I2C_DIRECTION_TRANSMITTER       ((uint8_t)0x00)
#define I2C_DIRECTION_RECEIVER          ((uint8_t)0x01)

void I2C_SoftWare_Master_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /*SDA GPIO clock enable */ //PB9
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
    /*SCL GPIO clock enable */ //PB6
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
    /* GPIO Configuration */
    /*Configure I2C SCL pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /*Configure I2C SDA pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SCL_H;
    SDA_H;
    
    SCL_L;
    SDA_L;
    
    SCL_H;
    SDA_H;
}

	
void I2C_delay(void)
{
    volatile int i = 10;		  
    while (i){
        i--;
        __asm("nop");
    }
}

uint8_t I2C_Start(void)
{
    SDA_H;
    SCL_H;
    I2C_delay();
    if (!SDA_read)
        return 0;
    SDA_L;
    I2C_delay();
    if (SDA_read)
        return 0;
    SCL_L;
    I2C_delay();
    return 1;
}

void I2C_Stop(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SDA_H;
    I2C_delay();
}


void I2C_Ack(void)
{
    SCL_L;
    I2C_delay();
    SDA_L;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

void I2C_NoAck(void)
{
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    SCL_L;
    I2C_delay();
}

uint8_t I2C_WaitAck(void)
{
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H; 
	 
    I2C_delay();
    if (SDA_read)
    {
        SCL_L;
        I2C_delay();
        return 0;
    }

    SCL_L;
    I2C_delay();
    
    return 1;
}

void I2C_SendByte(uint8_t byte)
{
    uint8_t i = 8;
    while (i--)
    {
        SCL_L;
        I2C_delay();
        if (byte & 0x80)
            SDA_H;
        else
            SDA_L;
        byte <<= 1;
        I2C_delay();
        SCL_H;
        I2C_delay();
    }
    SCL_L;
}

uint8_t I2C_ReceiveByte(void)
{
    uint8_t i = 8;
    uint8_t byte = 0;

    SDA_H;
    while (i--) 
    {
        byte <<= 1;
        SCL_L;
        I2C_delay();
        SCL_H;
        I2C_delay();
        if (SDA_read) 
	{
            byte |= 0x01;
        }
    }
    SCL_L;
    return byte;
}

int I2C_SoftWare_Master_Write(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t* pBuffer, uint16_t NumByteToWrite)
{
    int i;
    if (!I2C_Start())
        return I2C_SoftWare_Master_ReInit();
    I2C_SendByte(DeviceAddr | I2C_DIRECTION_TRANSMITTER);
    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return -1;
    }
    
    I2C_SendByte(RegAddr);
    I2C_WaitAck();
    
    for (i = 0; i < NumByteToWrite; i++) 
    {
        I2C_SendByte(pBuffer[i]);
        if (!I2C_WaitAck()) 
	{
            I2C_Stop();
            return I2C_SoftWare_Master_ReInit();
        }
    }
    
    I2C_Stop();
    
    return 0; 
}

int I2C_SoftWare_Master_Read(uint8_t DeviceAddr, uint8_t RegAddr, uint8_t* pBuffer, uint16_t NumByteToRead)
{
    if (!I2C_Start())
        return I2C_SoftWare_Master_ReInit();
    I2C_SendByte(DeviceAddr | I2C_DIRECTION_TRANSMITTER);
    if (!I2C_WaitAck())
    {
        I2C_Stop();
        return I2C_SoftWare_Master_ReInit();
    }
    I2C_SendByte(RegAddr);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(DeviceAddr | I2C_DIRECTION_RECEIVER);
    I2C_WaitAck();
    while (NumByteToRead) 
    {
        *pBuffer = I2C_ReceiveByte();
        if (NumByteToRead == 1)
            I2C_NoAck();
        else
            I2C_Ack();
        pBuffer++;
        NumByteToRead--;
    }
    I2C_Stop();
    
    return 0;
}

int I2C_SoftWare_Master_ReInit(void)
{
    I2C_SoftWare_Master_Init();
  
    return -1;
}
  
