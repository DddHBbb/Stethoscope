#include "GPIOConfig.h"


void ALL_GPIO_init(void)
{
		GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();      
    __HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
	
    GPIO_Initure.Pin=USB_Connect_Check_PIN;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
	
		GPIO_Initure.Pin=SD_STATUS_PIN;                                       
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
		GPIO_Initure.Pin=SD_SWITCH_PIN;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;              
    GPIO_Initure.Pull=GPIO_PULLUP; 
		GPIO_Initure.Speed = GPIO_SPEED_FAST;	
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
		GPIO_Initure.Pin=SPI_SWITCH_PIN;
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
		
		GPIO_Initure.Pin=OLED_SWITCH_PIN;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=RFID_SWITCH_PIN;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=BUlETHOOTH_SWITCH_PIN;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		GPIO_Initure.Pin=AUDIO_SWITCH_PIN;             
    GPIO_Initure.Pull=GPIO_PULLDOWN; 
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		/*µÁ≥ÿµÁ¡øºÏ≤‚GPIO*/
		GPIO_Initure.Pin=Batt_50|Batt_75|Batt_100;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=Batt_25;                                       
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);
		
	
}


















