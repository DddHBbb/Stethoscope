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
#include "D_delay.h"
#include "usart.h"
#include "string.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "string.h"
#include "sdio_sdcard.h"
#include "fontupd.h"
#include "text.h"
#include "wm8978.h"	 
#include "wavplay.h" 
#include "GPIOConfig.h"
#include "oled.h"
#include "spi.h"
#include "iwdg.h"
#include "key.h"  
#include "low_pwr.h"
#include "rtc.h"

/*
*************************************************************************
*                               函数声明
*************************************************************************
*/
void Timer_Init(void);
void Task_init(void);
void Semaphore_init(void);
void Mailbox_init(void);
void Event_init(void);
void rt_hw_us_delay(rt_uint32_t us);
char rt_hw_console_getchar(void);

#endif /* __BOARD_H__ */
