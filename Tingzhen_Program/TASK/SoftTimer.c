#include "SoftTimer.h"
#include "rtthread.h"
#include "board.h"

/***********************函数声明区*******************************/
static void LowPWR_timer_callback(void* parameter);

/***********************声明返回区*******************************/
extern rt_mailbox_t LOW_PWR_mb;
/***********************全局变量区*******************************/

rt_timer_t LowPWR_timer=RT_NULL;
/****************************************************************/
static void LowPWR_timer_callback(void* parameter) 
{ 
	static uint8_t count=0;
	if((rt_mb_recv(LOW_PWR_mb, RT_NULL, RT_WAITING_NO)) != RT_EOK)
	{
		count++;
		if(count == 20)
		{
			OLED_Clear();
			Show_String(36,36,(uint8_t*)"低功耗模式");
			rt_kprintf("进入低功耗\n");
			SysTick->CTRL = 0x00;//关闭定时器
			SysTick->VAL = 0x00;//清空val,清空定时器
			HAL_PWREx_EnableFlashPowerDown();
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);
			Stm32_Clock_Init(384,25,2,8);
			count=0;
			OLED_Clear();
			Show_String(0,0,(uint8_t*)"模拟听诊器");
			Show_String(0,12,(uint8_t*)"当前播放：");	
			Show_String(48,36,(uint8_t*)"停止播放");
			rt_kprintf("退出低功耗\n");
//		rt_kprintf("当前时基=%d\n",rt_tick_get());
		}
	}
	else 
		count=0;
}
void Timer_Init(void)
{
	LowPWR_timer = rt_timer_create("LowPWR_timer",LowPWR_timer_callback,0,1000,RT_TIMER_FLAG_SOFT_TIMER|RT_TIMER_FLAG_PERIODIC);
//	rt_timer_start(LowPWR_timer);
	
}