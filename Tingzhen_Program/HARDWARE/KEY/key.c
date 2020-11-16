 #include "key.h"  

/**
  * @brief  配置按键用到的I/O口
  * @param  无
  * @retval 无
  */
void Key_GPIO_Config(void)
{
#ifdef HAL_Key
	
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
#else
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*开启按键端口的时钟*/
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK|KEY2_GPIO_CLK,ENABLE);
	
	//选择按键的引脚
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	// 设置按键的引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	//使用结构体初始化按键
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
	
	//选择按键的引脚
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN; 
	//设置按键的引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//使用结构体初始化按键
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);	
	#endif

}



uint8_t Key_Read(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	static uint8_t   Key_Press_Start=0;										//按键按下标志
	static uint8_t   Key_Press_Stop=0;										  //按键弹起标志
	static uint16_t  Key_Press_count=0;										//按键动作计数
	static uint32_t  Key_Press_Times=0;										//按键连击次数
	static uint8_t   Key_Press_End=0;											//按键连击结束标志
	
	#ifdef HAL_Key
if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)==0)							//电源键按下
	#else
if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin)==0)
	#endif
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
		return Key_Press_Times;
	}
	
/*********************************************END OF FILE**********************/
