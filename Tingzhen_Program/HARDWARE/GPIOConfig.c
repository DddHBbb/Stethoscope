#include "GPIOConfig.h"


void ALL_GPIO_init(void)
{
		GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();      
    
    GPIO_Initure.Pin=USB_Connect_Check_PIN;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(GPIOC,&GPIO_Initure);
	
}