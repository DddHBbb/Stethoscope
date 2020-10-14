#include "board.h"
#include "rtthread.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"
#include "usb_bsp.h"


/***********************����������*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
/***********************����������*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB״̬
extern vu8 bDeviceState;		//USB���� ���
/***********************ȫ�ֱ�����*******************************/
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;

static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;

/****************************************************************/


 /****************************************
  * @brief  ADC�ɼ�������
  * @param  ��
  * @retval ��
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
			MSC_BOT_Data=mymalloc(SRAMIN,MSC_MEDIA_PACKET);			//�����ڴ�
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
	Wav_Player = rt_thread_create( "Wav_Player_Task",              /* �߳����� */
                      Wav_Player_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      2048,                 /* �߳�ջ��С */
                      0,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",              /* �߳����� */
                      USB_Transfer_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      0,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);

}
void Semaphore_init(void)
{
	USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex",RT_IPC_FLAG_PRIO);
}
