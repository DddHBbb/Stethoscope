#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
#include "main.h"

/** @addtogroup BSP
  * @{
  */

/** @defgroup X_NUCLEO_NFC03A1_Spi
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/** @defgroup X_NUCLEO_NFC03A1_Spi_Exported_Constants
  * @{
  */
#define SPI_RESPONSEBUFFER_SIZE		        255
/** 
 * @brief  SPI Interface pins 
 */ 
#define RFTRANS_95HF_SPI		        SPI3

#define RFTRANS_95HF_SPI_SCK_PIN                GPIO_PIN_3                
#define RFTRANS_95HF_SPI_SCK_GPIO_PORT          GPIOB                  
	 
#define RFTRANS_95HF_SPI_MISO_PIN               GPIO_PIN_4
#define RFTRANS_95HF_SPI_MISO_GPIO_PORT         GPIOB

#define RFTRANS_95HF_SPI_MOSI_PIN               GPIO_PIN_6
#define RFTRANS_95HF_SPI_MOSI_GPIO_PORT         GPIOD                

#define RFTRANS_95HF_SPI_NSS_PIN             	GPIO_PIN_7                  
#define RFTRANS_95HF_SPI_NSS_GPIO_PORT       	GPIOD                       


/** 
 * @brief  External Interupt pin PA9
 */ 
 /*
  IRQ_IN (CR95HF Pin 12)
 */
#define IRQIN_RFTRANS_95HF_PIN                  GPIO_PIN_0
#define IRQIN_RFTRANS_GPIO_PORT                 GPIOE  
	
/** 
 * @brief  IRQout Interface pin PA10
 */ 
 	/* 
	IRQ_OUT (CR95HF Pin 14)
	*/
#define IRQOUT_RFTRANS_95HF_PIN        		GPIO_PIN_5
#define IRQOUT_RFTRANS_95HF_PORT       		GPIOB

#define INTERFACE_PIN             		GPIO_PIN_7
#define INTERFACE_GPIO_PORT       		GPIOC
/** 
 * @brief  ST95HF Macro definition 
 */
/* set state on SPI_NSS pin */
#define RFTRANS_95HF_NSS_LOW() 			HAL_GPIO_WritePin(RFTRANS_95HF_SPI_NSS_GPIO_PORT, RFTRANS_95HF_SPI_NSS_PIN, GPIO_PIN_RESET)
#define RFTRANS_95HF_NSS_HIGH()  		HAL_GPIO_WritePin(RFTRANS_95HF_SPI_NSS_GPIO_PORT, RFTRANS_95HF_SPI_NSS_PIN, GPIO_PIN_SET)
/* set state on IRQ_In pin */
#define RFTRANS_95HF_IRQIN_LOW() 		HAL_GPIO_WritePin(IRQIN_RFTRANS_GPIO_PORT, IRQIN_RFTRANS_95HF_PIN, GPIO_PIN_RESET)	
#define RFTRANS_95HF_IRQIN_HIGH()		HAL_GPIO_WritePin(IRQIN_RFTRANS_GPIO_PORT, IRQIN_RFTRANS_95HF_PIN, GPIO_PIN_SET)
/* set state on SPI_CLK pin */
#define RFTRANS_95HF_SCK_LOW() 			HAL_GPIO_WritePin(RFTRANS_95HF_SPI_SCK_GPIO_PORT, RFTRANS_95HF_SPI_SCK_PIN, GPIO_PIN_RESET)
#define RFTRANS_95HF_SCK_HIGH()  		HAL_GPIO_WritePin(RFTRANS_95HF_SPI_SCK_GPIO_PORT, RFTRANS_95HF_SPI_SCK_PIN, GPIO_PIN_SET)
/**
  * @}
  */
  
/* Exported functions ------------------------------------------------------- */
/** @defgroup X_NUCLEO_NFC03A1_Spi_Exported_Functions
  * @{
  */
void RFTRANS_SPI_Init(void);
void RFTRANS_SPI_MSPInit(void);
void RFTRANS_ConfigureInterfacePin(void);
void SPI_SendReceiveBuffer(const uint8_t *pCommand, uint8_t length, uint8_t *pResponse);
uint8_t SPI_SendReceiveByte(uint8_t data); 
extern SPI_HandleTypeDef SPI1_Handler;  //SPI¾ä±ú

void SPI1_Init(void);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);
void SPI3_Init(void);
u8 SPI1_ReadWriteByte(u8 TxData);
#endif
