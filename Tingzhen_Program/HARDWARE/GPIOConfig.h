#ifndef __GPIOCONFIG_H
#define __GPIOCONFIG_H
#include "stm32f4xx.h"


#define 	USB_Connect_Check_PIN  					GPIO_PIN_7
#define 	SD_SWITCH_PIN  									GPIO_PIN_8
#define 	SD_STATUS_PIN  									GPIO_PIN_12
#define   SPI_SWITCH_PIN									GPIO_PIN_4
#define  	OLED_SWITCH_PIN									GPIO_PIN_6
#define   RFID_SWITCH_PIN									GPIO_PIN_7
#define   BUlETHOOTH_SWITCH_PIN						GPIO_PIN_12
#define   AUDIO_SWITCH_PIN								GPIO_PIN_11
#define		Batt_25													GPIO_PIN_8
#define		Batt_50													GPIO_PIN_15
#define		Batt_75													GPIO_PIN_14
#define		Batt_100												GPIO_PIN_13

#define 	ENABLE_ALL_SWITCH() 		 HAL_GPIO_WritePin(GPIOC,SPI_SWITCH_PIN,GPIO_PIN_RESET);\
																	 HAL_GPIO_WritePin(GPIOB,OLED_SWITCH_PIN|RFID_SWITCH_PIN,GPIO_PIN_RESET);\
																	 HAL_GPIO_WritePin(GPIOA,SD_SWITCH_PIN,GPIO_PIN_RESET);\
																	 HAL_GPIO_WritePin(GPIOE,BUlETHOOTH_SWITCH_PIN,GPIO_PIN_RESET);\
																	 HAL_GPIO_WritePin(GPIOE,AUDIO_SWITCH_PIN,GPIO_PIN_SET);
																	 
#define 	DISABLE_ALL_SWITCH() 		 HAL_GPIO_WritePin(GPIOC,SPI_SWITCH_PIN,GPIO_PIN_SET);\
																	 HAL_GPIO_WritePin(GPIOB,OLED_SWITCH_PIN|RFID_SWITCH_PIN,GPIO_PIN_SET);\
																	 HAL_GPIO_WritePin(GPIOA,SD_SWITCH_PIN,GPIO_PIN_SET);\
																	 HAL_GPIO_WritePin(GPIOE,BUlETHOOTH_SWITCH_PIN,GPIO_PIN_SET);\
																	 HAL_GPIO_WritePin(GPIOE,AUDIO_SWITCH_PIN,GPIO_PIN_RESET);
void ALL_GPIO_init(void);

#endif













