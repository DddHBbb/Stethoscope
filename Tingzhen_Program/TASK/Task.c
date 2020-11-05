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
static void Status_Reflash_Task(void* parameter);
/***********************声明返回区*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况
extern uint8_t AbortWavplay_Event_Flag;
/***********************全局变量区*******************************/
static 	char *NFC_TagID_RECV=NULL;
static 	char *Rev_From_BT=NULL;
static 	char *The_Auido_Name=NULL;
static 	char Last_Audio_Name[50]="noway";
static	rt_uint32_t rev_data=0;	

//任务句柄
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;
static rt_thread_t OLED_Display = RT_NULL;
static rt_thread_t NFC_Transfer = RT_NULL;
static rt_thread_t Status_Refresh = RT_NULL;
//信号量句柄
static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;
//邮箱句柄
rt_mailbox_t NFC_TagID_mb = RT_NULL;
rt_mailbox_t AbortWavplay_mb = RT_NULL;
rt_mailbox_t BuleTooth_Transfer_mb = RT_NULL;
rt_mailbox_t NFC_SendMAC_mb = RT_NULL;
rt_mailbox_t The_Auido_Name_mb = RT_NULL;
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
	static uint8_t DataToBT[19];
	printf("Wav_Player_Task\n");
	while(1)
	{
		if(!(rt_mb_recv(NFC_TagID_mb, (rt_uint32_t*)&NFC_TagID_RECV, RT_WAITING_NO)))
		{
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);			
			//记录最后一次播放的文件名		
			if(Compare_string((const char*)Last_Audio_Name,(const char*)NFC_TagID_RECV) != 1)
			{
				//发送当前位置信息
				rt_sprintf((char*)DataToBT,"Position:%s\r\n",NFC_TagID_RECV);
				HAL_UART_Transmit(&UART3_Handler, (uint8_t *)DataToBT,sizeof(DataToBT),1000); 
				while(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_TC)!=SET);		//等待发送结束
				rt_kprintf("DataToBT=%s\n",DataToBT);
				Buff_Clear((uint8_t*)DataToBT);
			}
			strcpy((char *)&Last_Audio_Name,(const char*)NFC_TagID_RECV);
			if(!(rt_mb_recv(The_Auido_Name_mb, (rt_uint32_t*)&The_Auido_Name, RT_WAITING_NO)))
			{
			  rt_event_send(Display_NoAudio,PLAYING);
				audio_play(Select_File(The_Auido_Name)); 
				rt_kprintf("The_Auido_Name=%s\n",The_Auido_Name);
				Buff_Clear((uint8_t*)The_Auido_Name);
			}
			rt_mutex_release(USBorAudioUsingSDIO_Mutex);	
//			rt_kprintf("播放次数=%d\n",t++);	
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
	static uint8_t DataToNFC[50];
	static uint8_t DataToPlayer[50];
	
	while(1)
	{
		rt_event_recv(Display_NoAudio,PLAYING|STOPPING,RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,RT_WAITING_NO,&rev_data);
		if(!rt_mb_recv(AbortWavplay_mb, RT_NULL, RT_WAITING_NO))
		{
			AbortWavplay_Event_Flag = 1;
		}
		else
		{
			AbortWavplay_Event_Flag = 0;	
		}			
		USART_RX_STA=0;
		if(!(rt_mb_recv(BuleTooth_Transfer_mb, (rt_uint32_t*)&Rev_From_BT, RT_WAITING_NO)))
		{
			//处理收到的数据
			rt_kprintf("Rev_From_BT =%s\n",Rev_From_BT);
			if(Rev_From_BT[0] == 'M' || Rev_From_BT[1] == 'a')
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[8]);i++)
				{
					DataToNFC[i] = Rev_From_BT[8+i];
				}
				rt_mb_send(NFC_SendMAC_mb,(rt_uint32_t )&DataToNFC);
			}
			else if(Rev_From_BT[0] == 'S' || Rev_From_BT[1] == 'o')
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[10]);i++)
				{
					DataToPlayer[i] = Rev_From_BT[10+i];
				}
				rt_mb_send(The_Auido_Name_mb,(rt_uint32_t)&DataToPlayer);
			}
		  Buff_Clear((uint8_t*)Rev_From_BT);
		}
		rt_thread_delay(1000);
			
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
		rt_thread_delay(1000);		
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
	HAL_GPIO_WritePin(GPIOE,BUlETHOOTH_SWITCH_PIN,GPIO_PIN_RESET);
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
	Wav_Player = rt_thread_create( "Wav_Player_Task",Wav_Player_Task,RT_NULL,2048,3,20);                                   
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
	Status_Refresh = rt_thread_create( "Status_Reflash",Status_Reflash_Task,RT_NULL,1024,2,20);                
  if (Status_Refresh != RT_NULL)    rt_thread_startup(Status_Refresh);
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
	 NFC_TagID_mb = rt_mb_create("NFC_TagID_mb",8,RT_IPC_FLAG_FIFO);
	 AbortWavplay_mb = rt_mb_create("AbortWavplay_mb",1,RT_IPC_FLAG_FIFO);
	 BuleTooth_Transfer_mb = rt_mb_create("BuleTooth_Transfer_mb",20,RT_IPC_FLAG_FIFO);
	 NFC_SendMAC_mb = rt_mb_create("NFC_SendMAC_mb",20,RT_IPC_FLAG_FIFO);
 	 The_Auido_Name_mb = rt_mb_create("The_Auido_Name_mb",20,RT_IPC_FLAG_FIFO);
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

















