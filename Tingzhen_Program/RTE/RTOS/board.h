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
*                               ��������
*************************************************************************
*/
void Task_init(void);
void Semaphore_init(void);


#endif /* __BOARD_H__ */
