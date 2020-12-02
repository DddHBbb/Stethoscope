#include "low_pwr.h"
#include "iwdg.h"

void Key_EXTI_Config(void)
{	
	GPIO_InitTypeDef GPIO_ST;
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_ST.Pin = GPIO_PIN_0;
	GPIO_ST.Mode = GPIO_MODE_IT_RISING;
	GPIO_ST.Pull = GPIO_PULLDOWN;
	GPIO_ST.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB,&GPIO_ST);
	
	HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI0_IRQn,0,0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	
}
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
  {		
//		HAL_PWR_DisableSleepOnExit();
		printf("触发中断\n");
  }
	printf("触发中断1\n");
}