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

/*��������궨��*/
#define Creat_Wav_Player
#define Creat_USB_Transfer
//#define Creat_OLED_Display
#define Creat_NFC_Transfer
#define Creat_Dispose
/*����״̬*/
#define  PLAYING   0x01
#define  STOPPING (0x01<<1)

/***********************����������*******************************/
static void Wav_Player_Task(void* parameter);
static void USB_Transfer_Task(void* parameter);
static void OLED_Display_Task(void* parameter);
static void NFC_Transfer_Task(void* parameter);
static void Dispose_Task(void* parameter);
/***********************����������*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB״̬
extern vu8 bDeviceState;		//USB���� ���
extern uint8_t AbortWavplay_Event_Flag;
/***********************ȫ�ֱ�����*******************************/
static 	char *NFCTag_UID_RECV=NULL;
static 	char *NFCTag_CustomID_RECV=NULL;
static 	char *Rev_From_BT=NULL;
static 	char *The_Auido_Name=NULL;
static 	char Last_Audio_Name[50]="noway";

//������
static rt_thread_t Wav_Player = RT_NULL;
static rt_thread_t USB_Transfer = RT_NULL;
//static rt_thread_t OLED_Display = RT_NULL;
static rt_thread_t NFC_Transfer = RT_NULL;
static rt_thread_t Dispose = RT_NULL;
//�ź������
static rt_mutex_t USBorAudioUsingSDIO_Mutex = RT_NULL;
//������
rt_mailbox_t NFC_TagID_mb = RT_NULL;
rt_mailbox_t AbortWavplay_mb = RT_NULL;
rt_mailbox_t BuleTooth_Transfer_mb = RT_NULL;
rt_mailbox_t NFC_SendMAC_mb = RT_NULL;
rt_mailbox_t The_Auido_Name_mb = RT_NULL;
rt_mailbox_t NFCTag_CustomID_mb = RT_NULL;
rt_mailbox_t Loop_PlayBack_mb = RT_NULL;
//�¼����
///rt_event_t Display_NoAudio = RT_NULL;
rt_event_t AbortWavplay_Event = RT_NULL;
rt_event_t PlayWavplay_Event = RT_NULL;
rt_event_t Prevent_Accidental_Play_Event = RT_NULL;
/****************************************************************/
uint8_t TT2Tag[NFCT2_MAX_TAGMEMORY];
char dataOut[COM_XFER_SIZE]; 

 /****************************************
  * @brief  WAV��Ƶ���ź���
  * @param  ��
  * @retval ��
  ***************************************/
void Wav_Player_Task(void* parameter)
{	
	static uint8_t DataToBT[17];
	printf("Wav_Player_Task\n");
	while(1)
	{
		//���ܼ�⵽NFC��ǩ������²ſ��Բ�����Ƶ
		if((rt_mb_recv(NFCTag_CustomID_mb, (rt_uint32_t*)&NFCTag_CustomID_RECV, RT_WAITING_NO)) == RT_EOK)
		{
			//��������usb����Ƶ���Ŷ���Ҫʹ��SDIO��Ϊ��ֹ��ͻֻ����һ��
			rt_mutex_take(USBorAudioUsingSDIO_Mutex,RT_WAITING_FOREVER);	
			//��¼���һ�β��ŵ��ļ�	
			if((rt_mb_recv(NFC_TagID_mb, (rt_uint32_t*)&NFCTag_UID_RECV, RT_WAITING_NO))== RT_EOK)
			{
				if(Compare_string((const char*)Last_Audio_Name,(const char*)NFCTag_UID_RECV) != 1)
				{
					//���͵�ǰλ����Ϣ
					rt_sprintf((char*)DataToBT,"Position:%x%x%x%x\r\n",NFCTag_CustomID_RECV[0],NFCTag_CustomID_RECV[1],NFCTag_CustomID_RECV[2],NFCTag_CustomID_RECV[3]);//11
					HAL_UART_Transmit(&UART3_Handler, (uint8_t *)DataToBT,sizeof(DataToBT),1000); 
					while(__HAL_UART_GET_FLAG(&UART3_Handler,UART_FLAG_TC)!=SET);		//�ȴ����ͽ���
					rt_kprintf("DataToBT=%s\n",DataToBT);
					Buff_Clear((uint8_t*)DataToBT);
				}
			}
			strcpy((char *)&Last_Audio_Name,(const char*)NFCTag_UID_RECV);
			//������Ƶ�ļ��������䣬ÿ��ֻ����һ��
			if((rt_mb_recv(The_Auido_Name_mb, (rt_uint32_t*)&The_Auido_Name, RT_WAITING_NO))== RT_EOK)
			{	
				Show_String(48,36,(uint8_t*)"���ڲ���");	
				rt_thread_delay(10);
				//���ÿ���ѭ������
				while((rt_mb_recv(Loop_PlayBack_mb, RT_NULL, RT_WAITING_NO)) == RT_EOK)
				{
					WM8978_Write_Reg(2,0x180);			
					audio_play(The_Auido_Name); 
					WM8978_Write_Reg(2,0x40);
					WM8978_HPvol_Set(20,20);
					rt_thread_delay(1);
				}		
				rt_thread_delay(10);
				Show_String(48,36,(uint8_t*)"ֹͣ����");
				rt_kprintf("The_Auido_Name=%s\n",The_Auido_Name);
				Buff_Clear((uint8_t*)The_Auido_Name);
			}
			rt_mutex_release(USBorAudioUsingSDIO_Mutex);	
		}	
		rt_thread_delay(10); //1s
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
		
	OLED_Clear();
	Show_String(0,0,(uint8_t*)"ģ��������");
	Show_String(0,12,(uint8_t*)"��ǰ���ţ�");	
	Show_String(48,36,(uint8_t*)"ֹͣ����");
	while(1)
	{		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{			
			rt_thread_suspend(Wav_Player);
			rt_thread_suspend(NFC_Transfer);
			SD_Init();
			OLED_Clear();
			Show_String(0,0,(uint8_t*)"ģ��������");
			Show_String(4,12,(uint8_t*)"��ǰ���ţ�");	
			Show_String(48,36,(uint8_t*)"USBģʽ");		
			MSC_BOT_Data=(uint8_t *)rt_malloc(MSC_MEDIA_PACKET);			//�����ڴ�
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
					BattChek();//��ֹ��˸
					Show_String(0,0,(uint8_t*)"ģ��������");
					Show_String(4,12,(uint8_t*)"��ǰ���ţ�");	
					Show_String(48,36,(uint8_t*)"ֹͣ����");			
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
		/*��ⲻ��NFC��ǩʱֹͣ����*/				
		USART_RX_STA=0;		//�������״̬��������ջ������
		/*�������յ������ݣ��������ﴦ��*/
		if(!(rt_mb_recv(BuleTooth_Transfer_mb, (rt_uint32_t*)&Rev_From_BT, RT_WAITING_NO)))
		{
			rt_kprintf("Rev_From_BT =%s\n",Rev_From_BT);
			if(Rev_From_BT[0] == 'M' || Rev_From_BT[1] == 'a')//mac��ַ
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[8]);i++)
				{
					DataToNFC[i] = Rev_From_BT[8+i];
				}
				rt_mb_send(NFC_SendMAC_mb,(rt_uint32_t )&DataToNFC);
			}
			else if(Rev_From_BT[0] == 'S' || Rev_From_BT[1] == 'o')//�����ļ���
			{
				for(int i=0;i<rt_strlen(&Rev_From_BT[10]);i++)
				{
					DataToPlayer[i] = Rev_From_BT[10+i];
				}
				rt_mb_send(The_Auido_Name_mb,(rt_uint32_t)&DataToPlayer);
			}
			else if(Rev_From_BT[0] == 'B' || Rev_From_BT[1] == 'T')//��������״̬
			{
				if(Rev_From_BT[2] == 'C')				BTS=1;				
				else if(Rev_From_BT[2] == 'D')	BTS=0;
			}
		  Buff_Clear((uint8_t*)Rev_From_BT);//��������ֹ���
		}
		if(BTS) BluetoothDisp(1);
		else		BluetoothDisp(0);
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_SET)	BattChek();	
		if((HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) == GPIO_PIN_RESET))
		{
			DISABLE_ALL_SWITCH();
			delay_ms(1000);//����ػ�����һ���͹ػ�		
			__set_FAULTMASK(1);
				NVIC_SystemReset();		
		}	
		IWDG_Feed();
		rt_thread_delay(100);
			
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
	
	while(1)
	{					
		rt_thread_delay(1000);	
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
	if( !demoIni() )platformLog("Initialization failed..\r\n");	//��ʼ��cr95hf
	HAL_GPIO_WritePin(GPIOE,BUlETHOOTH_SWITCH_PIN,GPIO_PIN_RESET);//��������
	ConfigManager_HWInit();//��ʼ������
	while(1)
	{
		demoCycle();
		rt_thread_delay(5); //1ms
	}
}
 /****************************************
  * @brief  ���񴴽�����������������ڴ˴����� 
  * @param  ��
  * @retval ��
  ***************************************/
void Task_init(void)
{
	#ifdef  Creat_Wav_Player
	/*���ֲ���������*/
	Wav_Player = rt_thread_create( "Wav_Player_Task",Wav_Player_Task,RT_NULL,2048,1,100);                                   
  if (Wav_Player != RT_NULL)    rt_thread_startup(Wav_Player);
	#endif
	#ifdef  Creat_USB_Transfer
	/*USB�������洢����*/
	USB_Transfer = rt_thread_create( "USB_Transfer_Task",USB_Transfer_Task,RT_NULL,512,1,20);                 
  if (USB_Transfer != RT_NULL)    rt_thread_startup(USB_Transfer);
	 #endif
	#ifdef  Creat_OLED_Display
	/*OLED��ʾ����*/
	OLED_Display = rt_thread_create( "OLED_Display_Task",OLED_Display_Task,RT_NULL,512,1,20);               
  if (OLED_Display != RT_NULL)    rt_thread_startup(OLED_Display);
	 #endif
	#ifdef  Creat_NFC_Transfer
	/*NFC����*/
	NFC_Transfer = rt_thread_create( "NFC_Transfer_Task",NFC_Transfer_Task,RT_NULL,1024,1,20);                 
  if (NFC_Transfer != RT_NULL)    rt_thread_startup(NFC_Transfer);
	 #endif
	#ifdef  Creat_Dispose
	Dispose = rt_thread_create( "Status_Reflash",Dispose_Task,RT_NULL,1024,2,20);                
  if (Dispose != RT_NULL)    rt_thread_startup(Dispose);
	#endif
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
	 NFCTag_CustomID_mb = rt_mb_create("NFCTag_CustomID_mb",4,RT_IPC_FLAG_FIFO);
	 NFC_TagID_mb = rt_mb_create("NFC_TagID_mb",4,RT_IPC_FLAG_FIFO);
	 AbortWavplay_mb = rt_mb_create("AbortWavplay_mb",1,RT_IPC_FLAG_FIFO);
	 BuleTooth_Transfer_mb = rt_mb_create("BuleTooth_Transfer_mb",20,RT_IPC_FLAG_FIFO);
	 NFC_SendMAC_mb = rt_mb_create("NFC_SendMAC_mb",20,RT_IPC_FLAG_FIFO);
 	 The_Auido_Name_mb = rt_mb_create("The_Auido_Name_mb",20,RT_IPC_FLAG_FIFO);
	 Loop_PlayBack_mb = rt_mb_create("Loop_PlayBack_mb",1,RT_IPC_FLAG_FIFO);
}
 /****************************************
  * @brief  �¼��������� 
  * @param  ��
  * @retval ��
  ***************************************/
void Event_init(void)
{
//	 Display_NoAudio = rt_event_create("Display_NoAudio",RT_IPC_FLAG_FIFO);
	AbortWavplay_Event = rt_event_create("AbortWavplay_Event",RT_IPC_FLAG_FIFO);
	PlayWavplay_Event = rt_event_create("PlaytWavplay_Event",RT_IPC_FLAG_FIFO);
	Prevent_Accidental_Play_Event = rt_event_create("Prevent_Accidental_Play_Event",RT_IPC_FLAG_FIFO);
}

















