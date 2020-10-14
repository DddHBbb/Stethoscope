#include "board.h"
#include "rtthread.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"
#include "usb_bsp.h"


/***********************函数声明区*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
/***********************声明返回区*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况
/***********************全局变量区*******************************/
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;

static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;

/****************************************************************/


 /****************************************
  * @brief  ADC采集处理函数
  * @param  无
  * @retval 无
  ***************************************/
void Wav_Player_Task(void* parameter)
{	
	while(1)
	{
		rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);
		audio_play();
		rt_mutex_release(USBorAudioUsingSDIO_Mutex);
			printf("laile");
		rt_thread_delay(1000); //1s
	}
}

void USB_Transfer_Task(void* parameter)
{		
	while(1)
	{		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{
			SD_Init();		
			MSC_BOT_Data=mymalloc(SRAMIN,MSC_MEDIA_PACKET);			//申请内存
			USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
			bDeviceState;
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);
			while(1)
			{
				if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_SET)
				{
					rt_mutex_release(USBorAudioUsingSDIO_Mutex);
					usbd_CloseMassStorage(&USB_OTG_dev);
					myfree(SRAMIN,MSC_BOT_Data);
					break;
				}
				rt_thread_delay(1000);  //100ms
			}
		}
			rt_thread_delay(100);  //10ms
	}
}
void Task_init(void)
{
	Wav_Player = rt_thread_create( "Wav_Player_Task",              /* 线程名字 */
                      Wav_Player_Task,   				 /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      2048,                 /* 线程栈大小 */
                      0,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",              /* 线程名字 */
                      USB_Transfer_Task,   				 /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      0,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);

}
void Semaphore_init(void)
{
	USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex",RT_IPC_FLAG_PRIO);
}
