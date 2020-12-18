#include "usart.h"
#include "logger.h"

extern rt_mailbox_t BuleTooth_Transfer_mb;
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

u8 aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
volatile u8 aRxBuffer1[2];
UART_HandleTypeDef UART1_Handler; //UART句柄
UART_HandleTypeDef UART3_Handler; //UART句柄

//初始化IO 串口1 
//bound:波特率
void uart_init(void)
{	
	//UART 初始化设置
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=115200;				    //波特率
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	UART1_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()会使能UART1
	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)&aRxBuffer1, 2);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
	
	UART3_Handler.Instance=USART3;					    //USART1
	UART3_Handler.Init.BaudRate=115200;				    //波特率
	UART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART3_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART3_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART3_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	UART3_Handler.Init.OverSampling = UART_OVERSAMPLING_8;
	HAL_UART_Init(&UART3_Handler);					    //HAL_UART_Init()会使能UART1
	HAL_UART_Receive_IT(&UART3_Handler, (u8 *)&aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
	
	logUsartInit(&UART1_Handler);
//  	__HAL_UART_CLEAR_FLAG(&UART3_Handler,UART_FLAG_RXNE);
}

//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FAST;		//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART1;	//复用为USART1
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA10
		
		HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	}
	if(huart->Instance==USART3)//如果是串口1，进行串口1 MSP初始化
	{	
		__HAL_RCC_USART3_CLK_ENABLE();			//使能USART1时钟
		__HAL_RCC_GPIOB_CLK_ENABLE();			//使能GPIOA时钟

	
		GPIO_Initure.Pin=GPIO_PIN_10;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_NOPULL;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;		//高速
		GPIO_Initure.Alternate=GPIO_AF7_USART3;	//复用为USART1
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin=GPIO_PIN_11;			//PA10
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   	//初始化PA10	
		
		HAL_NVIC_EnableIRQ(USART3_IRQn);		//使能USART1中断通道
		HAL_NVIC_SetPriority(USART3_IRQn, 0,0);	//抢占优先级3，子优先级3
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART3)//如果是串口3
	{
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else 
				{
					USART_RX_STA|=0x8000;	//接收完成了 
					rt_mb_send(BuleTooth_Transfer_mb,(rt_uint32_t )&USART_RX_BUF);
				}
			}
			else //还没收到0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
	}
}
 
//串口1中断服务程序
void USART3_IRQHandler(void)                	
{ 
	uint32_t timeout=0;
	uint32_t maxDelay=0xFFFF;
	uint8_t t=0;
	
	HAL_UART_IRQHandler(&UART3_Handler);	//调用HAL库中断处理公用函数
//	while (HAL_UART_GetState(&UART3_Handler) != HAL_UART_STATE_READY)//等待就绪
//	{		
////		t= UART3_Handler.Instance->SR;
////		t= UART3_Handler.Instance->DR;
////		printf("发生ORE溢出\n");
//		printf("UART3_Handler.State = %x\n",UART3_Handler.State);
//		UART3_Handler.State = HAL_UART_STATE_READY;
//		__HAL_UNLOCK(&UART3_Handler);
////	 __HAL_UART_CLEAR_FLAG(&UART3_Handler,UART_FLAG_RXNE);
////	 timeout++; //超时处理
////	 if(timeout>maxDelay) break;	
//	}
	timeout=0;
	while(HAL_UART_Receive_IT(&UART3_Handler, (uint8_t *)&aRxBuffer, RXBUFFERSIZE) != HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
		printf("UART3_Handler.State = %x\n",UART3_Handler.State);
		UART3_Handler.State = HAL_UART_STATE_READY;
		__HAL_UNLOCK(&UART3_Handler);
	 timeout++; //超时处理
	 if(timeout>maxDelay) break;	
	}
//	if((UART3_Handler.Instance->CR1 & 0x20)==0)
//		HAL_UART_Receive_IT(&UART3_Handler,(u8 *)&aRxBuffer,RXBUFFERSIZE);
	if(__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_ORE) != RESET)
		{
			if(UART3_Handler.ErrorCode == HAL_UART_ERROR_ORE)
			{
				t= UART3_Handler.Instance->SR;
				t= UART3_Handler.Instance->DR;
				printf("发生ORE溢出");
			}		
		}
/*	20201215晚调试中断接收卡死记录：
		
		现象：while (HAL_UART_GetState(&UART3_Handler) != HAL_UART_STATE_READY);
					上述代码getstate返回错误状态导致接收卡死
		
	1、最有可能是HAL库的bug，解决方法，注释掉lock和unlock相关代码。
	2、在发送函数中设置的timeout时间过长，导致新一轮接收来了，但发送还未结束，冲突。
	3、串口接收邮箱中的发送指针，用完后未指向NULL，导致溢出
		以上三种可能
*/	
} 
void USART1_IRQHandler(void)                	
{ 
	uint32_t timeout=0;
	uint32_t maxDelay=0x1FFFF;
	
	HAL_UART_IRQHandler(&UART1_Handler);	//调用HAL库中断处理公用函数
	timeout=0;
  while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)//等待就绪
	{
		timeout++;////超时处理
    if(timeout>maxDelay) break;		
	}   
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)&aRxBuffer1, 2) != HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
	  timeout++; //超时处理
	  if(timeout>maxDelay) break;	
	}
}
void Arry_Clear(uint8_t buf[],uint8_t len)
{
	for(int i=0;i<len;i++)
		{
			buf[i]=0; 
		}
}
void Pointer_Clear(uint8_t *buf)
{
	for(int i=0;i<80;i++)
		{
			buf[i]=0; 
		}
	if(buf != RT_NULL)
	{
		buf =RT_NULL;
	}
}






