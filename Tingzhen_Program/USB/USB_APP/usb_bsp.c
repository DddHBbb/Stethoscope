#include "usb_bsp.h"
#include "sys.h"  
#include "rtthread.h"
#include "usb_dcd.h"


//USB OTG 底层IO初始化
//pdev:USB OTG内核结构体指针
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
     GPIO_InitTypeDef  GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();                   //使能GPIOA时钟
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();              //使能OTG FS时钟

        //配置PA11 12
    GPIO_InitStruct.Pin=GPIO_PIN_11|GPIO_PIN_12;    //PA11 12
    GPIO_InitStruct.Mode=GPIO_MODE_AF_PP;           //复用
    GPIO_InitStruct.Pull=GPIO_NOPULL;               //无上下拉
    GPIO_InitStruct.Speed=GPIO_SPEED_HIGH;          //高速
    GPIO_InitStruct.Alternate=GPIO_AF10_OTG_FS;     //复用为OTG FS
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);         //初始化
}

//USB OTG 中断设置,开启USB FS中断
//pdev:USB OTG内核结构体指针
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{  	
    HAL_NVIC_SetPriority(OTG_FS_IRQn,0,3);          //抢占优先级0，子优先级3
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);                //使能OTG USB FS中断 
}

//USB OTG 中断设置,开启USB FS中断
//pdev:USB OTG内核结构体指针
void USB_OTG_BSP_DisableInterrupt(void)
{ 
		HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
}
//pdev:USB OTG内核结构体指针
//state:0,断电;1,上电
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{ 
}
//pdev:USB OTG内核结构体指针
void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{ 
} 
//USB_OTG us级延时函数
//usec:要延时的us数.
void USB_OTG_BSP_uDelay (const uint32_t usec)
{ 
  // 	delay_us(usec);
}
//USB_OTG ms级延时函数
//msec:要延时的ms数.
void USB_OTG_BSP_mDelay (const uint32_t msec)
{  
	//delay_ms(msec);
}
// 移除U盘设备
void usbd_CloseMassStorage(USB_OTG_CORE_HANDLE *pdev)
{
	DCD_DevDisconnect(pdev);

	USB_OTG_BSP_DisableInterrupt();	/* 关闭USB相关的所有中断 */
}

















