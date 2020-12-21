#include "rtc.h"
#include "rtthread.h"
#include "iwdg.h"
#include "board.h"

RTC_HandleTypeDef RTC_Handler;  //RTC句柄

void RTC_Init(void)
{     
  	RTC_Handler.Instance=RTC;
    RTC_Handler.Init.HourFormat=RTC_HOURFORMAT_24;//RTC设置为24小时格式 
    RTC_Handler.Init.AsynchPrediv=0X7F;           //RTC异步分频系数(1~0X7F)
    RTC_Handler.Init.SynchPrediv=0XFF;            //RTC同步分频系数(0~7FFF)   
    RTC_Handler.Init.OutPut=RTC_OUTPUT_DISABLE;     
    RTC_Handler.Init.OutPutPolarity=RTC_OUTPUT_POLARITY_HIGH;
    RTC_Handler.Init.OutPutType=RTC_OUTPUT_TYPE_OPENDRAIN;
    if(HAL_RTC_Init(&RTC_Handler)!=HAL_OK) 
			rt_kprintf("RTC失败\n");	
		 __HAL_RCC_RTC_DISABLE();
}
//RTC底层驱动，时钟配置
//此函数会被HAL_RTC_Init()调用
//hrtc:RTC句柄
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    __HAL_RCC_PWR_CLK_ENABLE();//使能电源时钟PWR
//		HAL_PWR_EnableBkUpAccess();//取消备份区域写保护
    
    RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_LSI;//LSE配置
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState=RCC_LSE_OFF;                  //RTC使用LSE
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    PeriphClkInitStruct.PeriphClockSelection=RCC_PERIPHCLK_RTC;//外设为RTC
    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_LSI;//RTC时钟源为LSE
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);     
    __HAL_RCC_RTC_ENABLE();//RTC时钟使能
	
		__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RTC_Handler, RTC_FLAG_WUTF);//清除RTC WAKE UP的标志
		HAL_RTCEx_SetWakeUpTimer_IT(&RTC_Handler,3,RTC_WAKEUPCLOCK_CK_SPRE_16BITS);    //设置重装载值和时钟 
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn,0x09,0); //抢占优先级1,子优先级2
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
//	  __HAL_RCC_RTC_DISABLE();
}
//RTC WAKE UP中断服务函数
void RTC_WKUP_IRQHandler(void)
{
    HAL_RTCEx_WakeUpTimerIRQHandler(&RTC_Handler); 
}

//RTC WAKE UP中断处理
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    rt_kprintf("RTC中断\n");
}




















































