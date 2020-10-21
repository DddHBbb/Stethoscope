#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

extern SPI_HandleTypeDef SPI1_Handler;  //SPI���

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);
void SPI3_Init(void);
u8 SPI1_ReadWriteByte(u8 TxData);
#endif
