#ifndef FLASH_H
#define FLASH_H

#include "sys.h"

uint16_t MyFlash_ReadHalfWord(uint32_t addr);
uint8_t MyFlash_ReadByte(uint32_t addr);
//都是半字
void MyFlash_Read(uint32_t addr,uint16_t *data,uint16_t length);
void MyFlash_Write(uint32_t addr,uint16_t *data,uint16_t length);
void MyFlash_Write_NoCheck(uint32_t addr,uint16_t *data,uint16_t length);
#endif

