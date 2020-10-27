#ifndef __BOARD_H__
#define __BOARD_H__

/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/
#include "rtthread.h"
/* STM32 �̼���ͷ�ļ� */
#include "stm32f4xx.h"

/* ������Ӳ��bspͷ�ļ� */
#include "usart.h"
#include "sys.h"
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


/*
*************************************************************************
*                               ��������
*************************************************************************
*/
void Task_init(void);
void Semaphore_init(void);
void Mailbox_init(void);
void Event_init(void);
void rt_hw_us_delay(rt_uint32_t us);


#endif /* __BOARD_H__ */
