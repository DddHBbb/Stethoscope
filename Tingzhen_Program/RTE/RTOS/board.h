#ifndef __BOARD_H__
#define __BOARD_H__

/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
#include "rtthread.h"
/* STM32 固件库头文件 */
#include "stm32f4xx.h"

/* 开发板硬件bsp头文件 */
#include "usart.h"
#include "sys.h"
#include "usart.h"
#include "string.h"
#include "malloc.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "string.h"
#include "sdio_sdcard.h"
#include "fontupd.h"
#include "text.h"
#include "wm8978.h"	 
#include "audioplay.h"
#include "GPIOConfig.h"
#include "oled.h"
/*
*************************************************************************
*                               函数声明
*************************************************************************
*/
void Task_init(void);
void Semaphore_init(void);


#endif /* __BOARD_H__ */
