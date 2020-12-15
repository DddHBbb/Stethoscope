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
		GPIO_Initure.Pin=OLED_SWITCH2_PIN;
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);
		
		GPIO_Initure.Pin=RFID_SWITCH_PIN;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=BUlETHOOTH_SWITCH_PIN;
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		GPIO_Initure.Pin=AUDIO_SWITCH_PIN;             
    GPIO_Initure.Pull=GPIO_PULLDOWN; 
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		DISABLE_ALL_SWITCH();
		
		 /*开关机 */
		GPIO_Initure.Pin=PBout_Pin|INT_Pin;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		GPIO_Initure.Pin=PSHOLD_Pin;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;              
    GPIO_Initure.Pull=GPIO_NOPULL; 
		GPIO_Initure.Speed = GPIO_SPEED_FAST;	
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
		
		
		/*电池电量检测GPIO*/
		GPIO_Initure.Pin=Batt_50|Batt_75|Batt_100;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=Batt_25;                                       
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);
		
		
		/*NFC   GPIO*/
		
		
		  /*Configure GPIO pins :  nSPI_SS_Pin */
		GPIO_Initure.Pin = nSPI_SS_Pin;
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_Initure.Pull = GPIO_NOPULL;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(nSPI_SS_GPIO_Port, &GPIO_Initure);

		/*Configure GPIO pin : SSI_0_Pin */
		GPIO_Initure.Pin = SSI_0_Pin;
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_Initure.Pull = GPIO_NOPULL;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(SSI_0_GPIO_Port, &GPIO_Initure);

		/*Configure GPIO pins : LED1_Pin nIRQ_IN_Pin */
		GPIO_Initure.Pin =nIRQ_IN_Pin;
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_Initure.Pull = GPIO_NOPULL;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(nIRQ_IN_GPIO_Port, &GPIO_Initure);

		/*Configure GPIO pin : nIRQ_OUT_Pin */
		GPIO_Initure.Pin = nIRQ_OUT_Pin;
		GPIO_Initure.Mode = GPIO_MODE_INPUT;
		GPIO_Initure.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(nIRQ_OUT_GPIO_Port, &GPIO_Initure);

		GPIO_Initure.Pin =nfc_pwr_Pin;
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_Initure.Pull = GPIO_PULLUP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(nfc_pwr_GPIO_Port, &GPIO_Initure);
		
		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(SSI_0_GPIO_Port, SSI_0_Pin, GPIO_PIN_SET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(nIRQ_IN_GPIO_Port, nIRQ_IN_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(nSPI_SS_GPIO_Port, nSPI_SS_Pin, GPIO_PIN_SET);
		
		HAL_GPIO_WritePin(nfc_pwr_GPIO_Port, nfc_pwr_Pin, GPIO_PIN_RESET);
		
	
}


















