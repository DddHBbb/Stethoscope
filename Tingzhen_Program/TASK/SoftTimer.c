#include "SoftTimer.h"
#include "rtthread.h"
#include "board.h"

/***********************����������*******************************/
static void LowPWR_timer_callback(void* parameter);

/***********************����������*******************************/
extern rt_mailbox_t LOW_PWR_mb;
/***********************ȫ�ֱ�����*******************************/

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
			Show_String(36,36,(uint8_t*)"�͹���ģʽ");
			rt_kprintf("����͹���\n");
			SysTick->CTRL = 0x00;//�رն�ʱ��
			SysTick->VAL = 0x00;//���val,��ն�ʱ��
			HAL_PWREx_EnableFlashPowerDown();
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_SLEEPENTRY_WFI);
			Stm32_Clock_Init(384,25,2,8);
			count=0;
			OLED_Clear();
			Show_String(0,0,(uint8_t*)"ģ��������");
			Show_String(0,12,(uint8_t*)"��ǰ���ţ�");	
			Show_String(48,36,(uint8_t*)"ֹͣ����");
			rt_kprintf("�˳��͹���\n");
//		rt_kprintf("��ǰʱ��=%d\n",rt_tick_get());
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