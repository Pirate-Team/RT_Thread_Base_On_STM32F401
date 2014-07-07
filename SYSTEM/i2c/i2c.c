#include "i2c.h"

uint16_t timeout;
#define TIMEOUT &timeout

#define SCL_H         GPIOB->BSRR = GPIO_Pin_6 /* GPIO_SetBits(GPIOB , GPIO_Pin_10)   */
#define SCL_L         GPIOB->BRR  = GPIO_Pin_6 /* GPIO_ResetBits(GPIOB , GPIO_Pin_10) */

#define SDA_H         GPIOB->BSRR = GPIO_Pin_7 /* GPIO_SetBits(GPIOB , GPIO_Pin_11)   */
#define SDA_L         GPIOB->BRR  = GPIO_Pin_7 /* GPIO_ResetBits(GPIOB , GPIO_Pin_11) */

#define SCL_read      GPIOB->IDR  & GPIO_Pin_6 /* GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_10) */
#define SDA_read      GPIOB->IDR  & GPIO_Pin_7 /* GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_11) */

static void I2C_delay(void)
{
    volatile int i = 5;  //到1都可以
    while (i)
        i--;
}

static bool I2C_Start(void)
{
    SDA_H;
    SCL_H;
    I2C_delay();
    if (!SDA_read)
        return false;
    SDA_L;
    I2C_delay();
    if (SDA_read)
        return false;
    SDA_L;
    I2C_delay();
    return true;
}

static void I2C_Stop(void)
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

static void I2C_Ack(void)
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

static void I2C_NoAck(void)
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

static bool I2C_WaitAck(void)
{
//	u8 n = 0;
    SCL_L;
    I2C_delay();
    SDA_H;
    I2C_delay();
    SCL_H;
    I2C_delay();
    if (SDA_read) {
        SCL_L;
        return false;
    }
/*    while(n<50)
    {
    	if(!SDA_read)
    	{
    		SCL_L;
    		return true;
    	}
    	n++;
    }*/
    SCL_L;
    return true;
}
//===================================================================================
//===================================================================================

/*********************************************************************
*Function: I2C_SendByte
*Description: 发送一字节数据
*Input: uint8_t byte 需发送的字节
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
static void I2C_SendByte(uint8_t byte)
{
    uint8_t i = 8;
    while (i--) {
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

/*********************************************************************
*Function: I2C_ReceiveByte
*Description: 接收一字节数据
*Input:
*Output:
*Return: 接收到的字节
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
static uint8_t I2C_ReceiveByte(void)
{
    uint8_t i = 8;
    uint8_t byte = 0;

    SDA_H;
    while (i--) {
        byte <<= 1;
        SCL_L;
        I2C_delay();
        SCL_H;
        I2C_delay();
        if (SDA_read) {
            byte |= 0x01;
        }
    }
    SCL_L;
    return byte;
}

//===================================================================================
//===================================================================================

/*********************************************************************
*Function: MyI2C_Init
*Description: i2c总线初始化
*Input:
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
void MyI2C_Init(void)
{
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	//已更改
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &gpio);
}

/*********************************************************************
*Function: MyI2C_WriteBuffer
*Description: 向从机写入数据
*Input: uint8_t addr 从机地址
*Input: uint8_t reg 从机寄存器起始地址
*Input: uint8_t len 数据长度，字节单位
*Input: uint8_t const *buf 数据指针
*Output:
*Return: 1 成功
*Return: 0 失败
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
bool MyI2C_WriteBuffer(uint8_t addr, uint8_t reg, uint8_t len, uint8_t const *buf)
{
    int i;
    if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    for (i = 0; i < len; i++) {
        I2C_SendByte(buf[i]);
        if (!I2C_WaitAck()) {
            I2C_Stop();
            return false;
        }
    }
    I2C_Stop();
    return true;
}

/*********************************************************************
*Function: MyI2C_Write
*Description:
*Input:
*Output:
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
bool MyI2C_Write(uint8_t addr, uint8_t reg, uint8_t const *data)
{
	return MyI2C_WriteBuffer(addr,reg,1,data);
}

/*********************************************************************
*Function: I2c_write
*Description: 用于MPU6050的i2c函数
*Description: 成功 0 失败 -1
*Input:
*Output:
*Return: 成功 0 失败 -1
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
bool I2c_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t const *buf)
{
	if(MyI2C_WriteBuffer(addr,reg,len,buf))
		return 0;
	else return -1;
}

/*********************************************************************
*Function: MyI2C_ReadBuffer
*Description: 从从机读取数据
*Input: uint8_t addr 从机地址
*Input: uint8_t reg 从机寄存器起始地址
*Input: uint8_t len 需读取的数据长度
*Output: uint8_t *buf 指向读取的数据
*Return:
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
bool MyI2C_ReadBuffer(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    I2C_Start();
    I2C_SendByte(addr << 1 | I2C_Direction_Receiver);
    I2C_WaitAck();
    while (len) {
        *buf = I2C_ReceiveByte();
        if (len == 1)
            I2C_NoAck();
        else
            I2C_Ack();
        buf++;
        len--;
    }
    I2C_Stop();
    return true;
}

bool MyI2C_Read(uint8_t addr, uint8_t reg, uint8_t *data)
{
	return MyI2C_ReadBuffer(addr,reg,1,data);
}

/*********************************************************************
*Function: I2c_read
*Description: 用于MPU6050的i2c函数
*Description: 成功 0 失败 -1
*Input:
*Output:
*Return: 成功 0 失败 -1
*Others:
*Author: Spacelan
*Date: 2014-2-25
*********************************************************************/
bool I2c_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	if(MyI2C_ReadBuffer(addr,reg,len,buf))
		return 0;
	else return -1;
}
/*
bool i2cWriteBit(uint8_t addr, uint8_t reg, uint8_t bitNum, uint8_t data)
{
    uint8_t b;
    uint16_t a;
    i2cRead(addr,reg,&b,TIMEOUT);
    a = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return i2cWrite(addr, reg,(uint8_t) a);
}

bool i2cWriteBits(uint8_t addr, uint8_t reg, uint8_t bitNum, uint8_t len, uint8_t data)
{
	uint8_t i;
	for(i=0;i<len;i++)
	{
		i2cWriteBit(addr,reg,bitNum-i,data & 1);
		data >>= 1;
	}
	return true;
}

bool i2cReadBit(uint8_t addr, uint8_t reg, uint8_t bitNum, uint8_t *data, uint16_t *timeout)
{
	uint8_t b;
	bool s = i2cRead(addr,reg,&b,timeout);
	*data = (b >> bitNum) & 1;
	return s;
}

bool i2cReadBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t *timeout)
{
    // 01101001 read byte
    // 76543210 bit numbers
    //    xxx   args: bitStart=4, length=3
    //    010   masked
    //   -> 010 shifted
    uint16_t count, b;
    if ((count = i2cRead(devAddr, regAddr, &b, timeout)) != 0) {
        uint16_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data =(uint8_t)  b;
    }
    return count;
}

bool i2cWriteWords(uint8_t addr, uint8_t reg, uint8_t len, uint16_t* data)
{
    int i;
    if (!I2C_Start())
        return false;
    I2C_SendByte(addr << 1 | I2C_Direction_Transmitter);
    if (!I2C_WaitAck()) {
        I2C_Stop();
        return false;
    }
    I2C_SendByte(reg);
    I2C_WaitAck();
    for (i = 0; i < len * 2; i++) {
        I2C_SendByte((uint8_t)(data[i] >> 8));
        if (!I2C_WaitAck()) {
            I2C_Stop();
            return false;
        }
        I2C_SendByte((uint8_t)data[i++]);
		if (!I2C_WaitAck()) {
			I2C_Stop();
			return false;
		}
    }
    I2C_Stop();
    return true;
}

bool i2cWriteWord(uint8_t addr, uint8_t reg, uint16_t data)
{
	return i2cWriteWords(addr,reg,1,&data);
}
*/




