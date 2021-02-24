#ifndef __KEY_H
#define	__KEY_H

#include "stm32f4xx.h"
#define    WAKEUP_PORT       GPIOC			   
#define    WAKEUP_PIN		     GPIO_PIN_5

#define    KEY_UP_PORT     	 GPIOB		   
#define    KEY_UP_PIN		   	 GPIO_PIN_1

#define    KEY_DOWN_PORT     GPIOB		   
#define    KEY_DOWN_PIN		   GPIO_PIN_2

#define    KEY_OK_PORT     	 GPIOE		   
#define    KEY_OK_PIN		   	 GPIO_PIN_7


#define KEY_ON							 1
#define KEY_OFF							 0

#define KEY0       					 HAL_GPIO_ReadPin(KEY_UP_PORT,KEY_UP_PIN)  //KEY0按键PH3
#define KEY1        				 HAL_GPIO_ReadPin(KEY_DOWN_PORT,KEY_DOWN_PIN)  //KEY1按键PH2
#define KEY2       					 HAL_GPIO_ReadPin(KEY_OK_PORT,KEY_OK_PIN) //KEY2按键PC13
#define WK_UP     				   HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)  //WKUP按键PA0

#define KEY_UP 							1
#define KEY_DOWN						2
#define KEY_OK							3
#define WKUP_PRES  				  4

void Key_GPIO_Config(void);
uint8_t KEY_Scan(void);
uint8_t Key_Read(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);

#endif /* __KEY_H */

