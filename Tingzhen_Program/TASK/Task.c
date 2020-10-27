#include "board.h"
#include "rtthread.h"
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
#include "usbd_msc_bot.h"
#include "usb_bsp.h"
#include "demo.h"
#include "platform.h"

#define  PLAYING   0x01
#define  STOPPING (0x01<<1)

/***********************����������*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
static void OLED_Display_Task(void* parameter);
static void NFC_Transfer_Task(void* parameter);
static void BuleTooth_Transfer_Task(void* parameter);
/***********************����������*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB״̬
extern vu8 bDeviceState;		//USB���� ���
extern uint8_t AbortWavplay_Event_Flag;
/***********************ȫ�ֱ�����*******************************/
static 	char *NFC_TagID_RECV=NULL;
//������
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;
static rt_thread_t OLED_Display = RT_NULL;
static rt_thread_t NFC_Transfer = RT_NULL;
static rt_thread_t BuleTooth_Transfer = RT_NULL;
//�ź������
static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;
//������
rt_mailbox_t NFC_TagID = RT_NULL;
rt_mailbox_t AbortWavplay_Event = RT_NULL;

//�¼����
rt_event_t Display_NoAudio = RT_NULL;
/****************************************************************/


 /****************************************
  * @brief  WAV��Ƶ���ź���
  * @param  ��
  * @retval ��
  ***************************************/
void Wav_Player_Task(void* parameter)
{	
	uint32_t t=0;
	printf("Wav_Player_Task\n");
	while(1)
	{
		if(!(rt_mb_recv(NFC_TagID, (rt_uint32_t*)&NFC_TagID_RECV, RT_WAITING_NO)))
		{
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);	
			rt_event_send(Display_NoAudio,PLAYING);
			audio_play(Select_File(NFC_TagID_RECV)); 
			rt_mutex_release(USBorAudioUsingSDIO_Mutex);	
			rt_kprintf("���Ŵ���=%d\n",t++);
		}	
		else
		{
			rt_event_send(Display_NoAudio,STOPPING);
		}
		rt_thread_delay(1); //1s
	}
}
 /****************************************
  * @brief  USB�������洢����
  * @param  ��
  * @retval ��
  ***************************************/
void USB_Transfer_Task(void* parameter)
{		
	printf("USB_Transfer_Task\n");
	while(1)
	{		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{
			SD_Init();		
			MSC_BOT_Data=(uint8_t *)rt_malloc(MSC_MEDIA_PACKET);			//�����ڴ�
			USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
			bDeviceState;
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);	
			rt_thread_suspend(Wav_Player);
			rt_thread_suspend(NFC_Transfer);	
			while(1)
			{
				if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_SET)
				{
					rt_mutex_release(USBorAudioUsingSDIO_Mutex);
					usbd_CloseMassStorage(&USB_OTG_dev);
					rt_free(MSC_BOT_Data);
					rt_thread_resume(Wav_Player);
					rt_thread_resume(NFC_Transfer);
					break;
				}
				rt_thread_delay(1000);  //100ms
			}
		}
			rt_thread_delay(1000);  //1000ms
	}
}
 /****************************************
  * @brief  OLED��ʾ����
  * @param  ��
  * @retval ��
  ***************************************/
void OLED_Display_Task(void* parameter)
{
	printf("OLED_Display_Task\n");	
	rt_uint32_t rev_data=0;
	while(1)
	{	
		rt_event_recv(Display_NoAudio,PLAYING|STOPPING,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,RT_WAITING_NO,&rev_data);
		if(!rt_mb_recv(AbortWavplay_Event, RT_NULL, RT_WAITING_NO))
			AbortWavplay_Event_Flag = 1;
		else
			AbortWavplay_Event_Flag = 0;		
		 rt_thread_delay(700); //1s
		
		OLED_Clear();		
		Show_String(0,0,"ģ��������");
		Show_String(0,12,"��ǰ���ţ� ");	
		if(rev_data == (PLAYING|STOPPING))
				Show_String(36,36,(uint8_t *)Select_File(NFC_TagID_RECV));	
		else if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)	
				Show_String(36,36,"USBģʽ ");
		else if(rev_data == STOPPING)
				Show_String(36,36,"����Ƶ");		
		BattChek();	

	}
}
 /****************************************
  * @brief  NFC���Ӵ�����
  * @param  ��
  * @retval ��
  ***************************************/
void NFC_Transfer_Task(void* parameter)
{
	printf("NFC_Transfer_Task\n");	
	if( !demoIni() )
	{
		platformLog("Initialization failed..\r\n");
	} 
	else
	{
		platformLog("Initialization succeeded..\r\n");
	}	
	while(1)
	{
		demoCycle();
		rt_thread_delay(1); //1ms
	}
	
}
 /****************************************
  * @brief  ���񴴽�����������������ڴ˴����� 
  * @param  ��
  * @retval ��
  ***************************************/
void Task_init(void)
{
	/*���ֲ���������*/
	Wav_Player = rt_thread_create( "Wav_Player_Task",              /* �߳����� */
                      Wav_Player_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      1024,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	
	/*USB�������洢����*/
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",              /* �߳����� */
                      USB_Transfer_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      0,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);
	 
	 	/*OLED��ʾ����*/
	OLED_Display = rt_thread_create( "OLED_Display_Task",              /* �߳����� */
                      OLED_Display_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (OLED_Display != RT_NULL)    rt_thread_startup(OLED_Display);
	 
	 	 	/*NFC����*/
	NFC_Transfer = rt_thread_create( "NFC_Transfer_Task",              /* �߳����� */
                      NFC_Transfer_Task,   				 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      1024,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (NFC_Transfer != RT_NULL)    rt_thread_startup(NFC_Transfer);
}
 /****************************************
  * @brief  �ź���������������ֵ������ 
  * @param  ��
  * @retval ��
  ***************************************/
void Semaphore_init(void)
{
	USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex",RT_IPC_FLAG_PRIO);
}
 /****************************************
	* @brief  ���䴴������  
  * @param  ��
  * @retval ��
  ***************************************/
void Mailbox_init(void)
{
	 NFC_TagID = rt_mb_create("NFC_TagID",8,RT_IPC_FLAG_FIFO);
	 AbortWavplay_Event = rt_mb_create("AbortWavplay_Event",1,RT_IPC_FLAG_FIFO);
//	 Display_NoAudio = rt_mb_create("Display_NoAudio",1,RT_IPC_FLAG_FIFO);
}
 /****************************************
  * @brief  �¼��������� 
  * @param  ��
  * @retval ��
  ***************************************/
void Event_init(void)
{
	 Display_NoAudio = rt_event_create("Display_NoAudio",RT_IPC_FLAG_FIFO);
}

















