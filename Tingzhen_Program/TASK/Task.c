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
#include "st25r95_com.h"

/*开启任务宏定义*/
#define Creat_Wav_Player
#define Creat_USB_Transfer
#define Creat_NFC_Transfer
#define Creat_UART_Dispose
#define Creat_Write_Card
#define Creat_Others

/***********************函数声明区*******************************/
static void Wav_Player_Task  		 (void *parameter);
static void USB_Transfer_Task		 (void *parameter);
static void NFC_Transfer_Task		 (void *parameter);
static void Dispose_Task     		 (void *parameter);
static void Dispose_Others_Task	 (void *parameter);
/***********************声明返回区*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG; //USB状态
extern vu8 bDeviceState;   //USB连接 情况
extern rt_timer_t LowPWR_timer;
extern void Adjust_Volume(void);
/***********************全局变量区*******************************/
static char *NFCTag_UID_RECV 						= NULL;
static char *NFCTag_CustomID_RECV			  = NULL;
static char *Rev_From_BT 								= NULL;
static char *The_Auido_Name 						= NULL;
char Last_Audio_Name[50] 								= "noway";
char dataOut[COM_XFER_SIZE] 						= {0};
uint8_t TT2Tag[NFCT2_MAX_TAGMEMORY] 		= {0};
const char ARM[][9]										  = {"24270042","24270043"};
const char PlayStatus[]									= "PlayStatus:Stop\r\n";
static uint8_t BTS                      = 0;

//任务句柄
static rt_thread_t Wav_Player 					= RT_NULL;
static rt_thread_t USB_Transfer 				= RT_NULL;
static rt_thread_t NFC_Transfer 				= RT_NULL;
static rt_thread_t UART_Dispose 				= RT_NULL;
static rt_thread_t Others 							= RT_NULL;

//信号量句柄
rt_mutex_t USBorAudioUsingSDIO_Mutex 		= RT_NULL;
rt_sem_t   Start_Play_sem								= RT_NULL;
rt_sem_t   Stop_Play_sem								= RT_NULL;
rt_sem_t   USB_Mode_sem								  = RT_NULL;

//邮箱句柄
rt_mailbox_t NFC_TagID_mb = RT_NULL;
rt_mailbox_t BuleTooth_Transfer_mb 			= RT_NULL;
rt_mailbox_t NFC_SendMAC_mb 						= RT_NULL;
rt_mailbox_t The_Auido_Name_mb 					= RT_NULL;
rt_mailbox_t NFCTag_CustomID_mb 				= RT_NULL;
rt_mailbox_t LOW_PWR_mb 								= RT_NULL;
rt_mailbox_t Start_Playing_mb 					= RT_NULL;
rt_mailbox_t Stop_Playing_mb 						= RT_NULL;

//事件句柄
rt_event_t AbortWavplay_Event 					= RT_NULL;
rt_event_t PlayWavplay_Event 						= RT_NULL;

/****************************************************************/

/****************************************
  * @brief  WAV音频播放函数
  * @param  无
  * @retval 无
  ***************************************/
void Wav_Player_Task(void *parameter)
{
    static uint8_t DataToBT[50] = {0};
    printf("Wav_Player_Task\n");

    while (1)
    {
        //加一层while是为抬起nfc不再发送位置信息
        while (1)
        {
            //在能检测到NFC标签的情况下才可以播放音频
            if ((rt_mb_recv(NFCTag_CustomID_mb, (rt_uint32_t *)&NFCTag_CustomID_RECV, RT_WAITING_NO)) == RT_EOK)
            {
                //互斥量，usb和音频播放都需要使用SDIO，为防止冲突只能用一个
                rt_mutex_take(USBorAudioUsingSDIO_Mutex, RT_WAITING_FOREVER);
                //记录最后一次播放的文件，相同读卡信息只发一次
                if ((rt_mb_recv(NFC_TagID_mb, (rt_uint32_t *)&NFCTag_UID_RECV, RT_WAITING_NO)) == RT_EOK)
                {
                    if (Compare_string((const char *)Last_Audio_Name, (const char *)NFCTag_UID_RECV) != 1)
                    {

                        //发送当前位置信息
                        rt_sprintf((char *)DataToBT, "Position:%x%x%02x%02x\r\n", NFCTag_CustomID_RECV[0], NFCTag_CustomID_RECV[1],
																																									NFCTag_CustomID_RECV[2], NFCTag_CustomID_RECV[3]); //11
                        rt_enter_critical();
                        HAL_UART_Transmit(&UART3_Handler, (uint8_t *)DataToBT, strlen((const char *)(DataToBT)), 1000);
                        while (__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_TC) != SET); //等待发送结束
                        rt_exit_critical();
                        rt_kprintf("DataToBT=%s\n", DataToBT);
                        Arry_Clear((uint8_t *)DataToBT, sizeof(DataToBT));
                    }
                }
                strcpy((char *)&Last_Audio_Name, (const char *)NFCTag_UID_RECV);								
                //接收音频文件名的邮箱，每次只接受一次
                if ((rt_mb_recv(The_Auido_Name_mb, (rt_uint32_t *)&The_Auido_Name, RT_WAITING_NO)) == RT_EOK)
                {	
										//配合血压手臂播放状态
										if (Compare_string(hex2Str((unsigned char*)NFCTag_CustomID_RECV,4),ARM[0]) == 1 || 
												Compare_string(hex2Str((unsigned char*)NFCTag_CustomID_RECV,4),ARM[1]) == 1)
										{ 
											while(1)
											{
												if ((rt_mb_recv(Start_Playing_mb, NULL, RT_WAITING_NO)) == RT_EOK)
												{			
													rt_mb_control(Stop_Playing_mb, RT_IPC_CMD_RESET, 0);//防止多次发送stop导致播放失败
													audio_play(The_Auido_Name); //不拿开就循环播放		
													HAL_UART_Transmit(&UART3_Handler, (uint8_t *)PlayStatus, strlen((const char *)(PlayStatus)), 1000);//向主机发送停止状态
													rt_mb_control(Start_Playing_mb, RT_IPC_CMD_RESET, 0);
													rt_sem_release(Stop_Play_sem);
												}					
												// 若检测到新的标签不为手臂标签，则退出
												if ((rt_mb_recv(NFC_TagID_mb, (rt_uint32_t *)&NFCTag_UID_RECV, RT_WAITING_NO)) == RT_EOK)
												{
														if ((Compare_string((const char *)Last_Audio_Name, (const char *)NFCTag_UID_RECV) != 1))												
																break;												
												}
												//检测到USB直接跳出
												if ((bDeviceState == GPIO_PIN_RESET) && HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_RESET)
														break;														
												rt_thread_delay(100);
											}
										}
										else						
											//正常播放状态
										{
											audio_play(The_Auido_Name);//不拿开就循环播放   	
											HAL_UART_Transmit(&UART3_Handler, (uint8_t *)PlayStatus, strlen((const char *)(PlayStatus)), 1000);//向主机发送停止状态
										}											
										rt_sem_release(Stop_Play_sem);
                    rt_kprintf("The_Auido_Name=%s\n", The_Auido_Name);
                    Pointer_Clear((uint8_t *)The_Auido_Name);
                    rt_mutex_release(USBorAudioUsingSDIO_Mutex);
										rt_mb_control(The_Auido_Name_mb, RT_IPC_CMD_RESET, 0); //清除邮箱状态
                    break;
                }
                rt_mutex_release(USBorAudioUsingSDIO_Mutex);
            }
            rt_thread_delay(100); //1s
        }
        rt_mb_control(NFCTag_CustomID_mb, RT_IPC_CMD_RESET, 0); //清除邮箱状态
        rt_thread_yield();
    }
}
/****************************************
  * @brief  USB大容量存储函数
  * @param  无
  * @retval 无
  ***************************************/
void USB_Transfer_Task(void *parameter)
{
    printf("USB_Transfer_Task\n");

    while (1)
    {
        if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_RESET)
        {
            rt_thread_suspend(Wav_Player);
            rt_thread_suspend(NFC_Transfer);
            rt_timer_stop(LowPWR_timer);

            SD_Init();
						rt_sem_release(USB_Mode_sem);
            MSC_BOT_Data = (uint8_t *)rt_malloc(MSC_MEDIA_PACKET); //申请内存
            USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_cb, &USR_cb);
            rt_mutex_take(USBorAudioUsingSDIO_Mutex, RT_WAITING_FOREVER);
            while (1)
            {
                ChargeDisplay();  
							  rt_thread_delay(100);
                if (bDeviceState == GPIO_PIN_RESET && (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_SET))
                {
                    rt_mutex_release(USBorAudioUsingSDIO_Mutex);
                    usbd_CloseMassStorage(&USB_OTG_dev);
                    rt_free(MSC_BOT_Data);

                    rt_thread_resume(Wav_Player);
                    rt_thread_resume(NFC_Transfer);
                    rt_timer_start(LowPWR_timer);
										rt_sem_release(Stop_Play_sem);
										rt_sem_control(USB_Mode_sem,RT_IPC_CMD_RESET,0);
                    break;
                }
            }
        }
        rt_thread_delay(1000); //1000ms
    }
}
void Dispose_Task(void *parameter)
{
    static uint8_t DataToNFC[50];
    static uint8_t DataToPlayer[100];
		int i=0;
	  HAL_GPIO_WritePin(GPIOE, BUlETHOOTH_SWITCH_PIN, GPIO_PIN_RESET); //开启蓝牙
	
    while (1)
    { 
					USART_RX_STA = 0; //清除接受状态，否则接收会出问题
        /*从蓝牙收到的数据，都在这里处理*/
        if (!(rt_mb_recv(BuleTooth_Transfer_mb, (rt_uint32_t *)&Rev_From_BT, RT_WAITING_FOREVER)))
        {
            rt_kprintf("Rev_From_BT =%s\n", Rev_From_BT);
            if (Rev_From_BT[0] == 'M' || Rev_From_BT[1] == 'a') //mac地址
            {
                for (int i = 0; i < rt_strlen(&Rev_From_BT[8]); i++)
                {
                    DataToNFC[i] = Rev_From_BT[8 + i];
                }
                rt_mb_send(NFC_SendMAC_mb, (rt_uint32_t)&DataToNFC);
            }
            else if (Rev_From_BT[0] == 'S' || Rev_From_BT[1] == 'o') //声音文件名
            {
                for (int i = 0; i < rt_strlen(&Rev_From_BT[10]); i++)
                {
                    DataToPlayer[i] = Rev_From_BT[10 + i];
                }
                rt_mb_send(The_Auido_Name_mb, (rt_uint32_t)&DataToPlayer);
            }
            else if (Rev_From_BT[0] == 'B' || Rev_From_BT[1] == 'T') //蓝牙连接状态
            {
                if 	(Rev_From_BT[2] == 'C')
                    BTS = 1;
                else if (Rev_From_BT[2] == 'D')
                    BTS = 0;
            }
						else if (Rev_From_BT[0] == 'P' || Rev_From_BT[1] == 'l')//接收停止开始命令
						{
								if (Rev_From_BT[12] == 'P')
										rt_mb_send(Start_Playing_mb, 1);
								else if (Rev_From_BT[12] == 'S')
									  rt_mb_send(Stop_Playing_mb,  1);	
								rt_kprintf("发送次数=%d\n",++i);
						}					
            Pointer_Clear((uint8_t *)Rev_From_BT); //清除指针防止溢出
            Arry_Clear(USART_RX_BUF, 80);
            rt_mb_control(BuleTooth_Transfer_mb, RT_IPC_CMD_RESET, 0); //清除邮箱状态
        }
    }
}
/****************************************
  * @brief  NFC连接处理函数
  * @param  无
  * @retval 无
  ***************************************/
void NFC_Transfer_Task(void *parameter)
{
    printf("NFC_Transfer_Task\n");
    if (!demoIni()) //初始化cr95hf
        platformLog("Initialization failed..\r\n");
    ConfigManager_HWInit();                                          //初始化读卡
    while (1)
    {
        demoCycle();
        Adjust_Volume();    //调整音量
        rt_thread_delay(1); //5ms
    }
}
void Dispose_Others_Task(void *parameter)
{
	printf("Dispose_Others_Task\n");
	while (1)
    {
				if (BTS)
            BluetoothDisp(1); //显示蓝牙连接状态
        else
            BluetoothDisp(0);
        if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_SET)
        {
            BattChek();
            OLED_Refresh_Gram();
        }
				if (rt_sem_take(Start_Play_sem,RT_WAITING_NO) == RT_EOK)
				{
						OLED_Clear();
						Show_String(0, 0,   (uint8_t *)"播放状态：");
						Show_String(32, 32, (uint8_t *)"正在播放");
						OLED_Refresh_Gram();
						rt_sem_control(Start_Play_sem,RT_IPC_CMD_RESET,0);
				}
				if (rt_sem_take(Stop_Play_sem, RT_WAITING_NO) == RT_EOK)
				{
						OLED_Clear();
						Show_String(0, 0,   (uint8_t *)"播放状态：");
						Show_String(32, 32, (uint8_t *)"停止播放");
						OLED_Refresh_Gram();
						rt_sem_control(Stop_Play_sem,RT_IPC_CMD_RESET,0);
				}
				if (rt_sem_take(USB_Mode_sem,  RT_WAITING_NO) == RT_EOK)
				{
						OLED_Clear();
            Show_String(0, 0, (uint8_t *)"播放状态：");
            Show_String(32, 32, (uint8_t *)"USB模式");
						OLED_Refresh_Gram();
				}
        Battery_Capacity_Transmit(); //电池电量上传
				
        rt_thread_delay(50); //5ms
    }
}
/****************************************
  * @brief  任务创建函数，所有任务均在此处创建 
  * @param  无
  * @retval 无
  ***************************************/
void Task_init(void)
{
#ifdef Creat_Wav_Player
    /*音乐播放器任务*/
    Wav_Player = rt_thread_create	 ("Wav_Player_Task", 	 Wav_Player_Task, 	 RT_NULL, 2048, 1, 50);
    if (Wav_Player != RT_NULL)
        rt_thread_startup(Wav_Player);
#endif
#ifdef Creat_USB_Transfer
    /*USB大容量存储任务*/
    USB_Transfer = rt_thread_create("USB_Transfer_Task", USB_Transfer_Task,  RT_NULL, 512,  0, 20);
    if (USB_Transfer != RT_NULL)
        rt_thread_startup(USB_Transfer);
#endif
#ifdef Creat_NFC_Transfer
    /*NFC任务*/
    NFC_Transfer = rt_thread_create("NFC_Transfer_Task", NFC_Transfer_Task,  RT_NULL, 1024, 1, 20);
    if (NFC_Transfer != RT_NULL)
        rt_thread_startup(NFC_Transfer);
#endif
		/*串口数据处理任务*/
#ifdef Creat_UART_Dispose
    UART_Dispose = rt_thread_create	("Dispose_Task", 	 Dispose_Task, 			 	 RT_NULL, 1024, 1, 20);
    if (UART_Dispose != RT_NULL)
        rt_thread_startup(UART_Dispose);
#endif
		/*其他数据处理任务*/
#ifdef Creat_Others
    Others = rt_thread_create		    ("Dipose_Others", 	 Dispose_Others_Task, RT_NULL, 512,  3, 20);
    if (Others != RT_NULL)
        rt_thread_startup(Others);
#endif
}
/****************************************
  * @brief  信号量创建函数，二值，互斥 
  * @param  无
  * @retval 无
  ***************************************/
void Semaphore_init(void)
{
    USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex", RT_IPC_FLAG_PRIO);
		Start_Play_sem						=	rt_sem_create("Start_Play_sem",1,RT_IPC_FLAG_PRIO);
		Stop_Play_sem             = rt_sem_create("Stop_Play_sem", 1,RT_IPC_FLAG_PRIO);
		USB_Mode_sem              = rt_sem_create("USB_Mode_sem",  1,RT_IPC_FLAG_PRIO);
	
		rt_sem_control(Start_Play_sem,RT_IPC_CMD_RESET,0);
//		rt_sem_control(Stop_Play_sem,RT_IPC_CMD_RESET,0);
		rt_sem_control(USB_Mode_sem,RT_IPC_CMD_RESET,0);
}
/****************************************
	* @brief  邮箱创建函数  
  * @param  无
  * @retval 无
  ***************************************/
void Mailbox_init(void)
{
    NFCTag_CustomID_mb 		= rt_mb_create("NFCTag_CustomID_mb", 		1, RT_IPC_FLAG_FIFO);
    NFC_TagID_mb 			 		= rt_mb_create("NFC_TagID_mb", 					1, RT_IPC_FLAG_FIFO);
    BuleTooth_Transfer_mb = rt_mb_create("BuleTooth_Transfer_mb", 1, RT_IPC_FLAG_FIFO);
    NFC_SendMAC_mb 				= rt_mb_create("NFC_SendMAC_mb", 				1, RT_IPC_FLAG_FIFO);
    The_Auido_Name_mb 		= rt_mb_create("The_Auido_Name_mb", 		1, RT_IPC_FLAG_FIFO);
    LOW_PWR_mb 						= rt_mb_create("LOW_PWR_mb", 						1, RT_IPC_FLAG_FIFO);
		Start_Playing_mb 			= rt_mb_create("Start_Playing_mb", 			1, RT_IPC_FLAG_FIFO);
	  Stop_Playing_mb 			= rt_mb_create("Stop_Playing_mb", 			1, RT_IPC_FLAG_FIFO);
}
/****************************************
  * @brief  事件创建函数 
  * @param  无
  * @retval 无
  ***************************************/
void Event_init(void)
{
    AbortWavplay_Event = rt_event_create("AbortWavplay_Event",  RT_IPC_FLAG_FIFO);
    PlayWavplay_Event  = rt_event_create("PlaytWavplay_Event",  RT_IPC_FLAG_FIFO);

}













