 #include "key.h"  

/**
  * @brief  ���ð����õ���I/O��
  * @param  ��
  * @retval ��
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
	
			/*��������*/
		GPIO_Initure.Pin=WAKEUP_PIN;
    GPIO_Initure.Mode=GPIO_MODE_INPUT;              
    GPIO_Initure.Pull=GPIO_PULLUP;                                       
    HAL_GPIO_Init(WAKEUP_PORT,&GPIO_Initure);	
#else
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*���������˿ڵ�ʱ��*/
	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK|KEY2_GPIO_CLK,ENABLE);
	
	//ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN; 
	// ���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	//ʹ�ýṹ���ʼ������
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);
	
	//ѡ�񰴼�������
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN; 
	//���ð���������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	//ʹ�ýṹ���ʼ������
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);	
	#endif

}



uint8_t Key_Read(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{
	static uint8_t   Key_Press_Start=0;										//�������±�־
	static uint8_t   Key_Press_Stop=0;										  //���������־
	static uint16_t  Key_Press_count=0;										//������������
	static uint32_t  Key_Press_Times=0;										//������������
	static uint8_t   Key_Press_End=0;											//��������������־
	
	#ifdef HAL_Key
if(HAL_GPIO_ReadPin(GPIOx, GPIO_Pin)==0)							//��Դ������
	#else
if(GPIO_ReadInputDataBit(GPIOx, GPIO_Pin)==0)
	#endif
		{
			//�������µ����������������
			if(Key_Press_Start==0)
			{
				Key_Press_Start=1;
				Key_Press_Stop=0;
				Key_Press_Times++;//���ذ�������
				Key_Press_count=0;
			}	
			if(Key_Press_Start==1)
			{
				Key_Press_count++;
				if(Key_Press_count>300)													//�������³�ʱ
				{					
					Key_Press_count=0;
					Key_Press_Start=0;	
					Key_Press_Stop=0;
				}	
			}		
		}
		else 
		{	
			//�������𵥻��������������
			if((Key_Press_Stop==0)&&(Key_Press_Start==1))
			{
				Key_Press_Stop=1;
				Key_Press_Start=0;
				Key_Press_count=0;
//				Key_Press_Times=0;//ֻ���ڷ���0��1������ɾ��
			}
			if(Key_Press_Stop==1)
			{				
				Key_Press_count++;			
				if(Key_Press_count>300)													//��������ʱ
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
