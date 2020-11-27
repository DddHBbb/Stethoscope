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
//#define Creat_OLED_Display
#define Creat_NFC_Transfer
#define Creat_Dispose
/*播放状态*/
#define  PLAYING   0x01
#define  STOPPING (0x01<<1)

/***********************函数声明区*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
static void OLED_Display_Task(void* parameter);
static void NFC_Transfer_Task(void* parameter);
static void Dispose_Task(void* parameter);
/***********************声明返回区*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB状态
extern vu8 bDeviceState;		//USB连接 情况
extern uint8_t AbortWavplay_Event_Flag;
/***********************全局变量区*******************************/
static 	char *NFCTag_UID_RECV=NULL;
static 	char *NFCTag_CustomID_RECV=NULL;
static 	char *Rev_From_BT=NULL;
static 	char *The_Auido_Name=NULL;
static 	char Last_Audio_Name[50]="noway";

//任务句柄
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;
//static rt_thread_t OLED_Display = RT_NULL;
static rt_thread_t NFC_Transfer = RT_NULL;
static rt_thread_t Dispose = RT_NULL;
//信号量句柄
static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;
//邮箱句柄
rt_mailbox_t NFC_TagID_mb = RT_NULL;
rt_mailbox_t AbortWavplay_mb = RT_NULL;
rt_mailbox_t BuleTooth_Transfer_mb = RT_NULL;
rt_mailbox_t NFC_SendMAC_mb = RT_NULL;
rt_mailbox_t The_Auido_Name_mb = RT_NULL;
rt_mailbox_t NFCTag_CustomID_mb = RT_NULL;
rt_mailbox_t Loop_PlayBack_mb = RT_NULL;
//事件句柄
///rt_event_t Display_NoAudio = RT_NULL;
rt_event_t AbortWavplay_Event = RT_NULL;
rt_event_t PlayWavplay_Event = RT_NULL;
rt_event_t Prevent_Accidental_Play_Event = RT_NULL;
/****************************************************************/
uint8_t TT2Tag[NFCT2_MAX_TAGMEMORY];
char dataOut[COM_XFER_SIZE]; 

 /****************************************
  * @brief  WAV音频播放函数
  * @param  无
  * @retval 无
  ***************************************/
void Wav_Player_Task(void* parameter)
{	
	static uint8_t DataToBT[17];
	printf("Wav_Player_Task\n");
	while(1)
	{
		//在能检测到NFC标签的情况下才可以播放音频
		if((rt_mb_recv(NFCTag_CustomID_mb, (rt_uint32_t*)&NFCTag_CustomID_RECV, RT_WAITING_NO)) == RT_EOK)
		{
			//互斥量，usb和音频播放都需要使用SDIO，为防止冲突只能用一个
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);	
			//记录最后一次播放的文件	
			if((rt_mb_recv(NFC_TagID_mb, (rt_uint32_t*)&NFCTag_UID_RECV, RT_WAITING_NO))== RT_EOK)
			{
				if(Compare_string((const char*)Last_Audio_Name,(const char*)NFCTag_UID_RECV) != 1)
				{
					//发送当前位置信息
					rt_sprintf((char*)DataToBT,"Position:%x%x%x%x\r\n",NFCTag_CustomID_RECV[0],NFCTag_CustomID_RECV[1],NFCTag_CustomID_RECV[2],NFCTag_CustomID_RECV[3]);//11
					HAL_UART_Transmit(&UART3_Handler, (uint8_t *)DataToBT,sizeof(DataToBT),1000); 
					while(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_TC)!=SET);		//等待发送结束
					rt_kprintf("DataToBT=%s\n",DataToBT);
					Buff_Clear((uint8_t*)DataToBT);
				}
			}
			strcpy((char *)&Last_Audio_Name,(const char*)NFCTag_UID_RECV);
			//接收音频文件名的邮箱，每次只接受一次
			if((rt_mb_recv(The_Auido_Name_mb, (rt_uint32_t*)&The_Auido_Name, RT_WAITING_NO))== RT_EOK)
			{	
				Show_String(48,36,(uint8_t*)"正在播放");	
				rt_thread_delay(10);
				//不拿开就循环播放
				while((rt_mb_recv(Loop_PlayBack_mb, RT_NULL, RT_WAITING_NO)) == RT_EOK)
				{
					WM8978_Write_Reg(2,0x180);			
					audio_play(The_Auido_Name); 
					WM8978_Write_Reg(2,0x40);
					WM8978_HPvol_Set(20,20);
					rt_thread_delay(1);
				}		
				rt_thread_delay(10);
				Show_String(48,36,(uint8_t*)"停止播放");
				rt_kprintf("The_Auido_Name=%s\n",The_Auido_Name);
				Buff_Clear((uint8_t*)The_Auido_Name);
			}
			rt_mutex_release(USBorAudioUsingSDIO_Mutex);	
		}	
		rt_thread_delay(10); //1s
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
		
	OLED_Clear();
	Show_String(0,0,(uint8_t*)"模拟听诊器");
	Show_String(0,12,(uint8_t*)"当前播放：");	
	Show_String(48,36,(uint8_t*)"停止播放");
	while(1)
	{		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{			
			rt_thread_suspend(Wav_Player);
			rt_thread_suspend(NFC_Transfer);
			SD_Init();
			OLED_Clear();
			Show_String(0,0,(uint8_t*)"模拟听诊器");
			Show_String(4,12,(uint8_t*)"当前播放：");	
			Show_String(48,36,(uint8_t*)"USB模式");		
			MSC_BOT_Data=(uint8_t *)rt_malloc(MSC_MEDIA_PACKET);			//申请内存
			USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);	
			while(1)
			{
				ChargeDisplay();
				delay_ms(500); 
				if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_SET)
				{
					rt_mutex_release(USBorAudioUsingSDIO_Mutex);
					usbd_CloseMassStorage(&USB_OTG_dev);
					rt_free(MSC_BOT_Data);
					rt_thread_resume(Wav_Player);
					rt_thread_resume(NFC_Transfer);
					OLED_Clear();
					BattChek();//防止闪烁
					Show_String(0,0,(uint8_t*)"模拟听诊器");
					Show_String(4,12,(uint8_t*)"当前播放：");	
					Show_String(48,36,(uint8_t*)"停止播放");			
					break;
				}
			}
		}
			rt_thread_delay(10);  //1000ms
	}
}
void Dispose_Task(void* parameter)
{
	static uint8_t DataToNFC[50];
	static uint8_t DataToPlayer[100];
	static uint8_t BTS=0;

	while(1)
	{
		/*检测不到NFC标签时停止播放*/				
		USART_RX_STA=0;		//清除接受状态，否则接收会出问题
		/*从蓝牙收到的数据，都在这里处理*/
		if(!(rt_mb_recv(BuleTooth_Transfer_mb, (rt_uint32_t*)&Rev_From_BT, RT_WAITING_NO)))
		{
			rt_kprintf("Rev_From_BT =%s\n",Rev_From_BT);
			if(Rev_From_BT[0] == 'M' || Rev_From_BT[1] == 'a')//mac地址
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[8]);i++)
				{
					DataToNFC[i] = Rev_From_BT[8+i];
				}
				rt_mb_send(NFC_SendMAC_mb,(rt_uint32_t )&DataToNFC);
			}
			else if(Rev_From_BT[0] == 'S' || Rev_From_BT[1] == 'o')//声音文件名
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[10]);i++)
				{
					DataToPlayer[i] = Rev_From_BT[10+i];
				}
				rt_mb_send(The_Auido_Name_mb,(rt_uint32_t)&DataToPlayer);
			}
			else if(Rev_From_BT[0] == 'B' || Rev_From_BT[1] == 'T')//蓝牙连接状态
			{
				if(Rev_From_BT[2] == 'C')				BTS=1;				
				else if(Rev_From_BT[2] == 'D')	BTS=0;
			}
		  Buff_Clear((uint8_t*)Rev_From_BT);//清除数组防止溢出
		}
		if(BTS) BluetoothDisp(1);
		else		BluetoothDisp(0);
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_SET)	BattChek();	
		if((HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) == GPIO_PIN_RESET))
		{
			DISABLE_ALL_SWITCH();
			delay_ms(1000);//慢点关机，别一轻点就关机		
			__set_FAULTMASK(1);
				NVIC_SystemReset();		
		}	
		IWDG_Feed();
		rt_thread_delay(100);
			
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
	
	while(1)
	{					
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
	if( !demoIni() )platformLog("Initialization failed..\r\n");	//初始化cr95hf
	HAL_GPIO_WritePin(GPIOE,BUlETHOOTH_SWITCH_PIN,GPIO_PIN_RESET);//开启蓝牙
	ConfigManager_HWInit();//初始化读卡
	while(1)
	{
		demoCycle();
		rt_thread_delay(5); //1ms
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
	Wav_Player = rt_thread_create( "Wav_Player_Task",Wav_Player_Task,RT_NULL,2048,1,100);                                   
  if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	#endif
	#ifdef  Creat_USB_Transfer
	/*USB大容量存储任务*/
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",USB_Transfer_Task,RT_NULL,512,1,20);                 
  if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);
	 #endif
	#ifdef  Creat_OLED_Display
	/*OLED显示任务*/
	OLED_Display = rt_thread_create( "OLED_Display_Task",OLED_Display_Task,RT_NULL,512,1,20);               
  if (OLED_Display != RT_NULL)    rt_thread_startup(OLED_Display);
	 #endif
	#ifdef  Creat_NFC_Transfer
	/*NFC任务*/
	NFC_Transfer = rt_thread_create( "NFC_Transfer_Task",NFC_Transfer_Task,RT_NULL,1024,1,20);                 
  if (NFC_Transfer != RT_NULL)    rt_thread_startup(NFC_Transfer);
	 #endif
	#ifdef  Creat_Dispose
	Dispose = rt_thread_create( "Status_Reflash",Dispose_Task,RT_NULL,1024,2,20);                
  if (Dispose != RT_NULL)    rt_thread_startup(Dispose);
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
	 NFCTag_CustomID_mb = rt_mb_create("NFCTag_CustomID_mb",4,RT_IPC_FLAG_FIFO);
	 NFC_TagID_mb = rt_mb_create("NFC_TagID_mb",4,RT_IPC_FLAG_FIFO);
	 AbortWavplay_mb = rt_mb_create("AbortWavplay_mb",1,RT_IPC_FLAG_FIFO);
	 BuleTooth_Transfer_mb = rt_mb_create("BuleTooth_Transfer_mb",20,RT_IPC_FLAG_FIFO);
	 NFC_SendMAC_mb = rt_mb_create("NFC_SendMAC_mb",20,RT_IPC_FLAG_FIFO);
 	 The_Auido_Name_mb = rt_mb_create("The_Auido_Name_mb",20,RT_IPC_FLAG_FIFO);
	 Loop_PlayBack_mb = rt_mb_create("Loop_PlayBack_mb",1,RT_IPC_FLAG_FIFO);
}
 /****************************************
  * @brief  事件创建函数 
  * @param  无
  * @retval 无
  ***************************************/
void Event_init(void)
{
//	 Display_NoAudio = rt_event_create("Display_NoAudio",RT_IPC_FLAG_FIFO);
	AbortWavplay_Event = rt_event_create("AbortWavplay_Event",RT_IPC_FLAG_FIFO);
	PlayWavplay_Event = rt_event_create("PlaytWavplay_Event",RT_IPC_FLAG_FIFO);
	Prevent_Accidental_Play_Event = rt_event_create("Prevent_Accidental_Play_Event",RT_IPC_FLAG_FIFO);
}

















