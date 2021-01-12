#include "spi.h"
#include "platform.h"
SPI_HandleTypeDef SPI1_Handler;  //SPI句柄
SPI_HandleTypeDef hspi3;

//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI5的初始化
void SPI1_Init(void)
{
    SPI1_Handler.Instance=SPI1;                         //SP1
    SPI1_Handler.Init.Mode=SPI_MODE_MASTER;             //设置SPI工作模式，设置为主模式
    SPI1_Handler.Init.Direction=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    SPI1_Handler.Init.DataSize=SPI_DATASIZE_8BIT;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI1_Handler.Init.CLKPolarity=SPI_POLARITY_HIGH;    //串行同步时钟的空闲状态为高电平
    SPI1_Handler.Init.CLKPhase=SPI_PHASE_2EDGE;         //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    SPI1_Handler.Init.NSS=SPI_NSS_SOFT;                 //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI1_Handler.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_256;//定义波特率预分频的值:波特率预分频值为256
    SPI1_Handler.Init.FirstBit=SPI_FIRSTBIT_MSB;        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI1_Handler.Init.TIMode=SPI_TIMODE_DISABLE;        //关闭TI模式
    SPI1_Handler.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    SPI1_Handler.Init.CRCPolynomial=7;                  //CRC值计算的多项式
    HAL_SPI_Init(&SPI1_Handler);
    
    __HAL_SPI_ENABLE(&SPI1_Handler);                    //使能SPI1
    SPI1_ReadWriteByte(0Xff);                           //启动传输
}
void SPI3_Init(void)
{
    hspi3.Instance=SPI3;                         //SP3
    hspi3.Init.Mode=SPI_MODE_MASTER;             //设置SPI工作模式，设置为主模式
    hspi3.Init.Direction=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    hspi3.Init.DataSize=SPI_DATASIZE_8BIT;       //设置SPI的数据大小:SPI发送接收8位帧结构
    hspi3.Init.CLKPolarity=SPI_POLARITY_HIGH;    //串行同步时钟的空闲状态为高电平
    hspi3.Init.CLKPhase=SPI_PHASE_2EDGE;         //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
    hspi3.Init.NSS=SPI_NSS_SOFT;                 //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    hspi3.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_16;//定义波特率预分频的值:波特率预分频值为256
    hspi3.Init.FirstBit=SPI_FIRSTBIT_MSB;        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    hspi3.Init.TIMode=SPI_TIMODE_DISABLE;        //关闭TI模式
    hspi3.Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    hspi3.Init.CRCPolynomial=7;                  //CRC值计算的多项式
    HAL_SPI_Init(&hspi3);
    
    __HAL_SPI_ENABLE(&hspi3);                    //使能SPI3
}
//SPI5底层驱动，时钟使能，引脚配置
//此函数会被HAL_SPI_Init()调用
//hspi:SPI句柄
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_Initure;
	if(hspi->Instance == SPI1)
	{    
    __HAL_RCC_GPIOA_CLK_ENABLE();       //使能GPIOA时钟
    __HAL_RCC_SPI1_CLK_ENABLE();        //使能SPI1时钟
    
    //PA7,5,6
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;             //快速            
    GPIO_Initure.Alternate=GPIO_AF5_SPI1;           //复用为SPI1
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	}
	if(hspi->Instance == SPI3)
	{
		__HAL_RCC_GPIOD_CLK_ENABLE();       //使能GPIOD时钟
		__HAL_RCC_GPIOB_CLK_ENABLE();       //使能GPIOB时钟
    __HAL_RCC_SPI3_CLK_ENABLE();        //使能SPI3时钟
		
		GPIO_Initure.Pin=GPIO_PIN_3|GPIO_PIN_4;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;             //快速            
    GPIO_Initure.Alternate=GPIO_AF6_SPI3;           //复用为SPI3
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
		
		GPIO_Initure.Pin=GPIO_PIN_6;
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;              //复用推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;                  //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;             //快速            
    GPIO_Initure.Alternate=GPIO_AF5_SPI3;           //复用为SPI3
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);	
	}
}

//SPI速度设置函数
//SPI速度=fAPB1/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
//fAPB1时钟一般为45Mhz：
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&SPI1_Handler);            //关闭SPI
    SPI1_Handler.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    SPI1_Handler.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&SPI1_Handler);             //使能SPI
    
}

//SPI5 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 Rxdata;
    HAL_SPI_TransmitReceive(&SPI1_Handler,&TxData,&Rxdata,1, 1000);       
 	return Rxdata;          		    //返回收到的数据		
}


uint8_t SPI_SendReceiveByte(uint8_t data) 
{	
  uint8_t ret_value;	
  
  HAL_SPI_TransmitReceive(&hspi3, (uint8_t*) &data, &ret_value, 1, 1000);
  
  return ret_value;
}

/**
 *	@brief  reveive a byte array over SPI
 *  @param  pCommand  : pointer on the buffer to send
 *  @param  length	 	: length of the buffer to send
 *  @param  pResponse : pointer on the buffer response
 *  @retval None 
 */
void SPI_SendReceiveBuffer(const uint8_t *pCommand, uint8_t length, uint8_t *pResponse) 
{
  uint8_t i;
  
  for(i=0; i<length; i++)
    pResponse[i] = SPI_SendReceiveByte(pCommand[i]);

}






