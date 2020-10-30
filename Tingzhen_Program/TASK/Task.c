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

/*开启任务宏定义*/
#define Creat_Wav_Player
#define Creat_USB_Transfer
#define Creat_OLED_Display
#define Creat_NFC_Transfer
#define Creat_Status_Reflash
/*播放状态*/
#define  PLAYING   0x01
#define  STOPPING (0x01<<1)

/***********************函数声明区*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
static void OLED_Display_Task(void* parameter);
static void NFC_Transfer_Task(void* parameter);
static void BuleTooth_Transfer_Task(void* parameter);
static void Status_Reflash_Task(void* parameter);
/***********************声明返回区*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况
extern uint8_t AbortWavplay_Event_Flag;
/***********************全局变量区*******************************/
static 	char *NFC_TagID_RECV=NULL;
static	rt_uint32_t rev_data=0;
//任务句柄
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;
static rt_thread_t OLED_Display = RT_NULL;
static rt_thread_t NFC_Transfer = RT_NULL;
static rt_thread_t BuleTooth_Transfer = RT_NULL;
static rt_thread_t Status_Reflash = RT_NULL;
//信号量句柄
static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;
//邮箱句柄
rt_mailbox_t NFC_TagID = RT_NULL;
rt_mailbox_t AbortWavplay_Event = RT_NULL;
//事件句柄
rt_event_t Display_NoAudio = RT_NULL;
/****************************************************************/


 /****************************************
  * @brief  WAV音频播放函数
  * @param  无
  * @retval 无
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
			rt_kprintf("播放次数=%d\n",t++);
		}	
		else
		{
			rt_event_send(Display_NoAudio,STOPPING);
		}
		rt_thread_delay(1); //1s
	}
}
 /****************************************
  * @brief  USB大容量存储函数
  * @param  无
  * @retval 无
  ***************************************/
void USB_Transfer_Task(void* parameter)
{		
	printf("USB_Transfer_Task\n");
	while(1)
	{		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{
			SD_Init();
			OLED_Clear();			
			MSC_BOT_Data=(uint8_t *)rt_malloc(MSC_MEDIA_PACKET);			//申请内存
			USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
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
					OLED_Clear();	
					break;
				}
				rt_thread_delay(1000);  //100ms
			}
		}
			rt_thread_delay(10);  //1000ms
	}
}
void Status_Reflash_Task(void* parameter)
{
	while(1)
	{
		rt_event_recv(Display_NoAudio,PLAYING|STOPPING,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,RT_WAITING_NO,&rev_data);
		if(!rt_mb_recv(AbortWavplay_Event, RT_NULL, RT_WAITING_NO))
			AbortWavplay_Event_Flag = 1;
		else
			AbortWavplay_Event_Flag = 0;		
			rt_thread_delay(500);
	}
}
 /****************************************
  * @brief  OLED显示函数
  * @param  无
  * @retval 无
  ***************************************/
void OLED_Display_Task(void* parameter)
{
	printf("OLED_Display_Task\n");
	OLED_Clear();	
	u8 len=20;
	while(1)
	{		
		Show_String(0,0,"模拟听诊器");
		Show_String(0,12,"当前播放： ");	
		if(rev_data == (PLAYING|STOPPING))
				Show_String(48,36,(uint8_t *)Select_File(NFC_TagID_RECV));	
		else if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)	
				Show_String(48,36,"USB模式");
		else if(rev_data == STOPPING)
				Show_String(48,36,"无播放");	
			BattChek();	
			rt_thread_delay(2000);
		
//	 if(USART_RX_STA&0x8000)
//		{					   
//			len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
//			printf("\r\n您发送的消息为:\r\n");
//			HAL_UART_Transmit(&UART1_Handler,(uint8_t*)USART_RX_BUF,len,1000);	//发送接收到的数据
//			while(__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_TC)!=SET);		//等待发送结束
//			printf("\r\n\r\n");//插入换行
//			USART_RX_STA=0;
//		}
		for(int i=0;i<USART_RX_STA;i++)
			{
				rt_kprintf("USART_RX_BUF = %c\n",USART_RX_BUF[i]);			
			}
			USART_RX_STA = 0;
			rt_thread_delay(2000);
	}
}
 /****************************************
  * @brief  NFC连接处理函数
  * @param  无
  * @retval 无
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
  * @brief  任务创建函数，所有任务均在此处创建 
  * @param  无
  * @retval 无
  ***************************************/
void Task_init(void)
{
	#ifdef  Creat_Wav_Player
	/*音乐播放器任务*/
	Wav_Player = rt_thread_create( "Wav_Player_Task",Wav_Player_Task,RT_NULL,1024,3,20);                                   
  if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	#endif
	#ifdef  Creat_USB_Transfer
	/*USB大容量存储任务*/
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",USB_Transfer_Task,RT_NULL,512,0,20);                 
  if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);
	 #endif
	#ifdef  Creat_OLED_Display
	/*OLED显示任务*/
	OLED_Display = rt_thread_create( "OLED_Display_Task",OLED_Display_Task,RT_NULL,512,1,20);               
  if (OLED_Display != RT_NULL)    rt_thread_startup(OLED_Display);
	 #endif
	#ifdef  Creat_NFC_Transfer
	/*NFC任务*/
	NFC_Transfer = rt_thread_create( "NFC_Transfer_Task",NFC_Transfer_Task,RT_NULL,1024,3,20);                 
  if (NFC_Transfer != RT_NULL)    rt_thread_startup(NFC_Transfer);
	 #endif
	#ifdef  Creat_Status_Reflash
	Status_Reflash = rt_thread_create( "Status_Reflash",Status_Reflash_Task,RT_NULL,512,2,20);                
  if (Status_Reflash != RT_NULL)    rt_thread_startup(Status_Reflash);
	#endif
}
 /****************************************
  * @brief  信号量创建函数，二值，互斥 
  * @param  无
  * @retval 无
  ***************************************/
void Semaphore_init(void)
{
	USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex",RT_IPC_FLAG_PRIO);
}
 /****************************************
	* @brief  邮箱创建函数  
  * @param  无
  * @retval 无
  ***************************************/
void Mailbox_init(void)
{
	 NFC_TagID = rt_mb_create("NFC_TagID",8,RT_IPC_FLAG_FIFO);
	 AbortWavplay_Event = rt_mb_create("AbortWavplay_Event",1,RT_IPC_FLAG_FIFO);
}
 /****************************************
  * @brief  事件创建函数 
  * @param  无
  * @retval 无
  ***************************************/
void Event_init(void)
{
	 Display_NoAudio = rt_event_create("Display_NoAudio",RT_IPC_FLAG_FIFO);
}

















