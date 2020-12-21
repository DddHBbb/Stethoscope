 #include "key.h"  

/**
  * @brief  配置按键用到的I/O口
  * @param  无
  * @retval 无
  */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOC_CLK_ENABLE();      
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

		/*开机检测键*/
	GPIO_Initure.Pin=WAKEUP_PIN;
	GPIO_Initure.Mode=GPIO_MODE_INPUT;              
	GPIO_Initure.Pull=GPIO_PULLUP;                                       
	HAL_GPIO_Init(WAKEUP_PORT,&GPIO_Initure);	
	
	GPIO_Initure.Pin = KEY_UP_PIN;
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(KEY_UP_PORT,&GPIO_Initure);	

	GPIO_Initure.Pin = KEY_DOWN_PIN;
	HAL_GPIO_Init(KEY_DOWN_PORT,&GPIO_Initure);	

}

uint8_t KEY_Scan(void)
{
	static uint8_t key_up=1;     //按键松开标志

	if(key_up&&(KEY0==1||KEY1==1||KEY2==1||WK_UP==1))
	{
		key_up=0;
		if(KEY0==1)       return KEY0_PRES;
		else if(KEY1==1)  return KEY1_PRES;
		else if(KEY2==1)  return KEY2_PRES;
		else if(WK_UP==1) return WKUP_PRES;          
	} 
	else if(KEY0==0&&KEY1==0&&KEY2==0&&WK_UP==0)key_up=1;
	return 0;   //无按键按下
}	


//只能作为单独按键
uint8_t Key_Read(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	static uint8_t   Key_Press_Start=0;										//按键按下标志
	static uint8_t   Key_Press_Stop=0;										  //按键弹起标志
	static uint16_t  Key_Press_count=0;										//按键动作计数
	static uint32_t  Key_Press_Times=0;										//按键连击次数
	static uint8_t   Key_Press_End=0;											//按键连击结束标志
if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)== GPIO_PIN_SET)					

		{
			//按键按下单击或连击动作检测
			if(Key_Press_Start==0)
			{
				Key_Press_Start=1;
				Key_Press_Stop=0;
				Key_Press_Times++;//返回按键次数
				Key_Press_count=0;
			}	
			if(Key_Press_Start==1)
			{
				Key_Press_count++;
				if(Key_Press_count>300)													//按键按下超时
				{					
					Key_Press_count=0;
					Key_Press_Start=0;	
					Key_Press_Stop=0;
				}	
			}		
		}
		else 
		{	
			//按键弹起单击或连击动作检测
			if((Key_Press_Stop==0)&&(Key_Press_Start==1))
			{
				Key_Press_Stop=1;
				Key_Press_Start=0;
				Key_Press_count=0;
//				Key_Press_Times=0;//只用于返回0，1，否则删掉
			}
			if(Key_Press_Stop==1)
			{				
				Key_Press_count++;			
				if(Key_Press_count>300)													//按键弹起超时
				{
					Key_Press_count=0;
					Key_Press_Start=0;	
					Key_Press_Stop=0;
					Key_Press_End=1;				
				}				
			}
		}
		if(Key_Press_Times>=4) 
			Key_Press_Times = 4;
		
		return Key_Press_Times;
	}




















/*********************************************END OF FILE**********************/
