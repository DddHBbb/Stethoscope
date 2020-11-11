/**
  ******************************************************************************
  * @file    main.h
  * @author  MMY Application Team
  * @version $Revision: 1507 $
  * @date    $Date: 2016-01-08 09:48:35 +0100 (Fri, 08 Jan 2016) $
  * @brief   This file provides all the headers of the main functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty  
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/

#include "common.h"
#include "com.h"

#include "lib_95HFConfigManager.h"
#include "mcc.h"

/* Exported types ------------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define BULK_MAX_PACKET_SIZE            0x00000040

/* Regarding board antenna (and matching) appropriate 
value may be modified to optimized RF performances */
/* Analogue configuration register
 ARConfigB	bits  7:4	MOD_INDEX	Modulation index to modulator
                      3:0	RX_AMP_GAIN	Defines receiver amplifier gain
For type A you can also adjust the Timer Window
*/

/******************  PCD  ******************/
/* ISO14443A */
#define PCD_TYPEA_ARConfigA	        0x01
#define PCD_TYPEA_ARConfigB	        0xDF

#define PCD_TYPEA_TIMERW                0x5A

/* ISO14443B */
#define PCD_TYPEB_ARConfigA	        0x01
#define PCD_TYPEB_ARConfigB	        0x51

/* Felica */
#define PCD_TYPEF_ARConfigA	        0x01
#define PCD_TYPEF_ARConfigB	        0x51

/* ISO15693 */
#define PCD_TYPEV_ARConfigA	        0x01
#define PCD_TYPEV_ARConfigB	        0xD1

/******************  PICC  ******************/
/* ISO14443A */
#define PICC_TYPEA_ACConfigA            0x27  /* backscaterring */

/* ISO14443B */
#define PICC_TYPEB_ARConfigD            0x0E  /* card demodulation gain */
#define PICC_TYPEB_ACConfigA            0x17  /* backscaterring */

/* Felica */
#define PICC_TYPEF_ACConfigA            0x17  /* backscaterring */

/* Exported constants --------------------------------------------------------*/
#ifndef USE_CR95HF_UART
#define SPI_INTERRUPT_MODE_ACTIVATED
#endif                   

/** 
 * @brief  TIMER definitions 
 */
 
/* -------------------------------------------------------------------------- 
* Delay TIMER configuration (ms)
* -------------------------------------------------------------------------- */ 
#define TIMER_DELAY			TIM2
#define TIMER_DELAY_PERIOD		72
#define TIMER_DELAY_PRESCALER		1000
																						
/* -------------------------------------------------------------------------- 
* Delay TIMER configuration (탎)
* --------------------------------------------------------------------------- */ 
#define TIMER_US_DELAY			TIM2
#define TIMER_US_DELAY_PERIOD		35
#define TIMER_US_DELAY_PRESCALER	1

/*----------------------------------------------------------------------------*/
#ifdef STM32F401xE

#define TIMER_TIMEOUT			TIM3
#define APPLI_TIMER_TIMEOUT		TIM4
#define TIMER_TIMEOUT_IRQ_CHANNEL	TIM3_IRQn
#define APPLI_TIMER_TIMEOUT_IRQ_CHANNEL	TIM4_IRQn
#define EXTI_RFTRANS_95HF_IRQ_CHANNEL	EXTI15_10_IRQn
#define TIMER_TIMEOUT_IRQ_HANDLER	TIM3_IRQHandler
#define APPLI_TIMER_TIMEOUT_IRQ_HANDLER	TIM4_IRQHandler
#define RFTRANS_95HF_IRQ_HANDLER	EXTI15_10_IRQHandler

#elif defined STM32F103xB

#define TIMER_TIMEOUT                   TIM3
#define APPLI_TIMER_TIMEOUT		TIM4
#define TIMER_TIMEOUT_IRQ_CHANNEL	TIM3_IRQn
#define APPLI_TIMER_TIMEOUT_IRQ_CHANNEL	TIM4_IRQn
#define EXTI_RFTRANS_95HF_IRQ_CHANNEL	EXTI15_10_IRQn
#define TIMER_TIMEOUT_IRQ_HANDLER	TIM3_IRQHandler
#define APPLI_TIMER_TIMEOUT_IRQ_HANDLER	TIM4_IRQHandler
#define RFTRANS_95HF_IRQ_HANDLER        EXTI15_10_IRQHandler			
          
#endif
/*----------------------------------------------------------------------------*/
          
/* -------------------------------------------------------------------------- 
* timeout configuration (ms)
* --------------------------------------------------------------------------
* 72 MHz / 72 = 1MHz (1us )
* 1탎 * 1000 + 1탎 ~= 1ms	
* -------------------------------------------------------------------------- */

#define TIMER_TIMEOUT_PERIOD		1000
#define TIMER_TIMEOUT_PRESCALER		72

/* -------------------------------------------------------------------------- 
* timeout configuration (ms)
* --------------------------------------------------------------------------
* 72 MHz / 72 = 1MHz (1us )
* 1탎 * 1000 + 1탎 ~= 1ms	
* -------------------------------------------------------------------------- */

#define APPLI_TIMER_TIMEOUT_PERIOD	1000
#define APPLI_TIMER_TIMEOUT_PRESCALER	72

/** 
 * @brief  NVIC definitions 
 */
 
 
#define COM_XFER_SIZE   256


/**
 *	@brief  Interrupts configuration
|---------------|-----------------------|-------------------|-------------------|
|	 name	|preemption priority	|sub proiority	    |	channel	        |
|---------------|-----------------------|-------------------|-------------------|
| 95HF		|	0		|	0           |	EXTI15_10_IRQn	|
|---------------|-----------------------|-------------------|-------------------|
| delay		|	0		|	0           |	TIM2_IRQn	|
|---------------|-----------------------|-------------------|-------------------|
| timeout	|	0		|	0           |	TIM3_IRQn	|
|---------------|-----------------------|-------------------|-------------------|
| appli timeout	|	0		|	0           |	TIM4_IRQn	|
|---------------|-----------------------|-------------------|-------------------|
 */
#define EXTI_RFTRANS_95HF_PREEMPTION_PRIORITY   1
#define EXTI_RFTRANS_95HF_SUB_PRIORITY		1

#define TIMER_DELAY_PREEMPTION_PRIORITY		1
#define TIMER_DELAY_SUB_PRIORITY		3
#define TIMER_DELAY_IRQ_CHANNEL			TIM2_IRQn
#define TIMER_DELAY_PREEMPTION_HIGHPRIORITY	0
#define TIMER_DELAY_SUB_HIGHPRIORITY		0

#define TIMER_TIMEOUT_PREEMPTION_PRIORITY	0
#define TIMER_TIMEOUT_SUB_PRIORITY		0

#define APPLI_TIMER_TIMEOUT_PREEMPTION_PRIORITY	0
#define APPLI_TIMER_TIMEOUT_SUB_PRIORITY	1

/** 
 * @brief  IRQ handler functions names  
 */
#define TIMER_DELAY_IRQ_HANDLER			TIM2_IRQHandler
        

/* Exported functions ------------------------------------------------------- */
void drvInt_Enable_Reply_IRQ(void);
void drvInt_Enable_RFEvent_IRQ(void);
void drvInt_Disable_95HF_IRQ(void);

/* Exported functions ------------------------------------------------------- */
void HAL_Delay_Us(uint16_t delay);

/* Timeout use during the communication with the 95HF device with SPI or UART path */
void StartTimeOut(uint16_t delay);
void StopTimeOut(void);
void StartAppliTimeOut(uint16_t delay);
void StopAppliTimeOut(void);

void Timer_Structure_Config(void);
void FillTagURI(char* pTagType);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
