#ifndef __KEY_H
#define	__KEY_H

#define HAL_Key
#ifdef  HAL_Key

#include "stm32f4xx.h"
#define    WAKEUP_PORT        GPIOC			   
#define    WAKEUP_PIN		      GPIO_PIN_5

#define    KEY2_GPIO_PORT     GPIOC		   
#define    KEY2_GPIO_PIN		  GPIO_PIN_13

#else
#include "stm32f10x.h"
//  Òý½Å¶¨Òå
#define    KEY1_GPIO_CLK     RCC_APB2Periph_GPIOE
#define    KEY1_GPIO_PORT    GPIOE			   
#define    KEY1_GPIO_PIN		 GPIO_Pin_2

#define    KEY2_GPIO_CLK     RCC_APB2Periph_GPIOC
#define    KEY2_GPIO_PORT    GPIOC		   
#define    KEY2_GPIO_PIN		  GPIO_Pin_13
#endif

#define KEY_ON	1
#define KEY_OFF	0

void Key_GPIO_Config(void);
uint8_t Key_Read(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);

#endif /* __KEY_H */

