#ifndef __SAI_H
#define __SAI_H
#include "sys.h"

extern SAI_HandleTypeDef SAI1A_Handler;        //SAI1 Block A句柄

extern void (*sai_tx_callback)(void);		//sai tx回调函数指针  

void SAIA_Init(u32 mode,u32 cpol,u32 datalen);
u8 SAIA_SampleRate_Set(u32 samplerate);
void SAIA_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num,u8 width);
void SAIA_DMA_Enable(void);
void SAI_Play_Start(void); 
void SAI_Play_Stop(void); 
#endif
