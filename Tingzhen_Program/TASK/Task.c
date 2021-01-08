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

/*��������궨��*/
#define Creat_Wav_Player
#define Creat_USB_Transfer
//#define Creat_OLED_Display
#define Creat_NFC_Transfer
#define Creat_Dispose

/***********************����������*******************************/
static void Wav_Player_Task  (void *parameter);
static void USB_Transfer_Task(void *parameter);
static void OLED_Display_Task(void *parameter);
static void NFC_Transfer_Task(void *parameter);
static void Dispose_Task     (void *parameter);
/***********************����������*******************************/
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG; //USB״̬
extern vu8 bDeviceState;   //USB���� ���
extern rt_timer_t LowPWR_timer;
extern void Adjust_Volume(void);
/***********************ȫ�ֱ�����*******************************/
static char *NFCTag_UID_RECV 						= NULL;
static char *NFCTag_CustomID_RECV			  = NULL;
static char *Rev_From_BT 								= NULL;
static char *The_Auido_Name 						= NULL;
char Last_Audio_Name[50] 								= "noway";
char dataOut[COM_XFER_SIZE] 						= {0};
uint8_t TT2Tag[NFCT2_MAX_TAGMEMORY] 		= {0};

//������
static rt_thread_t Wav_Player 					= RT_NULL;
static rt_thread_t USB_Transfer 				= RT_NULL;
//static rt_thread_t OLED_Display 			= RT_NULL;
static rt_thread_t NFC_Transfer 				= RT_NULL;
static rt_thread_t Dispose 							= RT_NULL;

//�ź������
rt_mutex_t USBorAudioUsingSDIO_Mutex 		= RT_NULL;

//������
rt_mailbox_t NFC_TagID_mb = RT_NULL;
//rt_mailbox_t AbortWavplay_mb 					= RT_NULL;
rt_mailbox_t BuleTooth_Transfer_mb 			= RT_NULL;
rt_mailbox_t NFC_SendMAC_mb 						= RT_NULL;
rt_mailbox_t The_Auido_Name_mb 					= RT_NULL;
rt_mailbox_t NFCTag_CustomID_mb 				= RT_NULL;
//rt_mailbox_t Loop_PlayBack_mb 				= RT_NULL;
rt_mailbox_t LOW_PWR_mb 								= RT_NULL;

//�¼����
///rt_event_t Display_NoAudio 					= RT_NULL;
rt_event_t AbortWavplay_Event 					= RT_NULL;
rt_event_t PlayWavplay_Event 						= RT_NULL;
rt_event_t OLED_Display_Event 					= RT_NULL;
/****************************************************************/

/****************************************
  * @brief  WAV��Ƶ���ź���
  * @param  ��
  * @retval ��
  ***************************************/
void Wav_Player_Task(void *parameter)
{
    static uint8_t DataToBT[50] = {0};
    static uint8_t tt;
    printf("Wav_Player_Task\n");

    while (1)
    {
        //��һ��while��Ϊ̧��nfc���ٷ���λ����Ϣ
        while (1)
        {
            //���ܼ�⵽NFC��ǩ������²ſ��Բ�����Ƶ
            if ((rt_mb_recv(NFCTag_CustomID_mb, (rt_uint32_t *)&NFCTag_CustomID_RECV, RT_WAITING_NO)) == RT_EOK)
            {
                //��������usb����Ƶ���Ŷ���Ҫʹ��SDIO��Ϊ��ֹ��ͻֻ����һ��
                rt_mutex_take(USBorAudioUsingSDIO_Mutex, RT_WAITING_FOREVER);
                //��¼���һ�β��ŵ��ļ�����ͬ������Ϣֻ��һ��
                if ((rt_mb_recv(NFC_TagID_mb, (rt_uint32_t *)&NFCTag_UID_RECV, RT_WAITING_NO)) == RT_EOK)
                {
                    if (Compare_string((const char *)Last_Audio_Name, (const char *)NFCTag_UID_RECV) != 1)
                    {

                        //���͵�ǰλ����Ϣ
                        rt_sprintf((char *)DataToBT, "Position:%x%x%02x%02x\r\n", NFCTag_CustomID_RECV[0], NFCTag_CustomID_RECV[1],
																																									NFCTag_CustomID_RECV[2], NFCTag_CustomID_RECV[3]); //11
                        rt_enter_critical();
                        HAL_UART_Transmit(&UART3_Handler, (uint8_t *)DataToBT, strlen((const char *)(DataToBT)), 1000);
                        while (__HAL_UART_GET_FLAG(&UART3_Handler, UART_FLAG_TC) != SET); //�ȴ����ͽ���
                        rt_exit_critical();
                        rt_kprintf("DataToBT=%s\n", DataToBT);
                        Arry_Clear((uint8_t *)DataToBT, sizeof(DataToBT));
                    }
                }
                strcpy((char *)&Last_Audio_Name, (const char *)NFCTag_UID_RECV);
								
                //������Ƶ�ļ��������䣬ÿ��ֻ����һ��
                if ((rt_mb_recv(The_Auido_Name_mb, (rt_uint32_t *)&The_Auido_Name, RT_WAITING_NO)) == RT_EOK)
                {
                    //���ÿ���ѭ������
                    audio_play(The_Auido_Name);                            //��ʼ������Ƶ�ļ�
                    rt_mb_control(The_Auido_Name_mb, RT_IPC_CMD_RESET, 0); //�������״̬
                    //��ֹ����
                    OLED_Clear();
                    Show_String(0, 0, (uint8_t *)"����״̬��");
                    Show_String(32, 32, (uint8_t *)"ֹͣ����");
                    BluetoothDisp(1);
                    BattChek();
                    OLED_Refresh_Gram();

                    rt_kprintf("The_Auido_Name=%s\n", The_Auido_Name);
                    Pointer_Clear((uint8_t *)The_Auido_Name);
                    rt_mutex_release(USBorAudioUsingSDIO_Mutex);
                    break;
                }
                rt_mutex_release(USBorAudioUsingSDIO_Mutex);
            }

            rt_thread_delay(100); //1s
        }
        rt_mb_control(NFCTag_CustomID_mb, RT_IPC_CMD_RESET, 0); //�������״̬
        rt_thread_yield();
    }
}
/****************************************
  * @brief  USB�������洢����
  * @param  ��
  * @retval ��
  ***************************************/
void USB_Transfer_Task(void *parameter)
{
    printf("USB_Transfer_Task\n");
    OLED_Clear();
    Show_String(0, 0, (uint8_t *)"����״̬��");
    Show_String(32, 32, (uint8_t *)"ֹͣ����");
    OLED_Refresh_Gram();

    while (1)
    {
        if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_RESET)
        {
            rt_thread_suspend(Wav_Player);
            rt_thread_suspend(NFC_Transfer);
            rt_timer_stop(LowPWR_timer);

            SD_Init();
            OLED_Clear();
            Show_String(0, 0, (uint8_t *)"����״̬��");
            Show_String(32, 32, (uint8_t *)"USBģʽ");
            MSC_BOT_Data = (uint8_t *)rt_malloc(MSC_MEDIA_PACKET); //�����ڴ�
            USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_cb, &USR_cb);
            rt_mutex_take(USBorAudioUsingSDIO_Mutex, RT_WAITING_FOREVER);
            while (1)
            {
                ChargeDisplay();
                OLED_Refresh_Gram(); //������ʾ
                if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_SET)
                {
                    rt_mutex_release(USBorAudioUsingSDIO_Mutex);
                    usbd_CloseMassStorage(&USB_OTG_dev);
                    rt_free(MSC_BOT_Data);

                    rt_thread_resume(Wav_Player);
                    rt_thread_resume(NFC_Transfer);
                    rt_timer_start(LowPWR_timer);

                    OLED_Clear();
                    BattChek(); //��ֹ��˸
                    Show_String(0, 0, (uint8_t *)"����״̬��");
                    Show_String(32, 32, (uint8_t *)"ֹͣ����");
                    OLED_Refresh_Gram();
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
    static uint8_t BTS = 0;
    static uint8_t OLED_Display_Flag = 0;
    static uint8_t volume = 85;

    while (1)
    {
        USART_RX_STA = 0; //�������״̬��������ջ������
        /*�������յ������ݣ��������ﴦ��*/
        if (!(rt_mb_recv(BuleTooth_Transfer_mb, (rt_uint32_t *)&Rev_From_BT, RT_WAITING_NO)))
        {
            rt_kprintf("Rev_From_BT =%s\n", Rev_From_BT);
            if (Rev_From_BT[0] == 'M' || Rev_From_BT[1] == 'a') //mac��ַ
            {
                for (int i = 0; i < rt_strlen(&Rev_From_BT[8]); i++)
                {
                    DataToNFC[i] = Rev_From_BT[8 + i];
                }
                rt_mb_send(NFC_SendMAC_mb, (rt_uint32_t)&DataToNFC);
            }
            else if (Rev_From_BT[0] == 'S' || Rev_From_BT[1] == 'o') //�����ļ���
            {
                for (int i = 0; i < rt_strlen(&Rev_From_BT[10]); i++)
                {
                    DataToPlayer[i] = Rev_From_BT[10 + i];
                }
                rt_mb_send(The_Auido_Name_mb, (rt_uint32_t)&DataToPlayer);
            }
            else if (Rev_From_BT[0] == 'B' || Rev_From_BT[1] == 'T') //��������״̬
            {
                if 	(Rev_From_BT[2] == 'C')
                    BTS = 1;
                else if (Rev_From_BT[2] == 'D')
                    BTS = 0;
            }
            Pointer_Clear((uint8_t *)Rev_From_BT); //���ָ���ֹ���
            Arry_Clear(USART_RX_BUF, 80);
            rt_mb_control(BuleTooth_Transfer_mb, RT_IPC_CMD_RESET, 0); //�������״̬
        }
        if (BTS)
            BluetoothDisp(1); //��ʾ��������״̬
        else
            BluetoothDisp(0);
        if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_SET)
        {
            BattChek();
            OLED_Refresh_Gram();
        }
        Battery_Capacity_Transmit(); //��ص����ϴ�
        rt_thread_delay(500);
    }
}
/****************************************
  * @brief  OLED��ʾ����
  * @param  ��
  * @retval ��
  ***************************************/
void OLED_Display_Task(void *parameter)
{
    printf("OLED_Display_Task\n");

    while (1)
    {
        rt_thread_delay(1000);
    }
}
/****************************************
  * @brief  NFC���Ӵ�����
  * @param  ��
  * @retval ��
  ***************************************/
void NFC_Transfer_Task(void *parameter)
{
    printf("NFC_Transfer_Task\n");
    if (!demoIni()) //��ʼ��cr95hf
        platformLog("Initialization failed..\r\n");
    HAL_GPIO_WritePin(GPIOE, BUlETHOOTH_SWITCH_PIN, GPIO_PIN_RESET); //��������
    ConfigManager_HWInit();                                          //��ʼ������
    while (1)
    {
        demoCycle();
        Adjust_Volume();    //��������
        rt_thread_delay(5); //5ms
    }
}
/****************************************
  * @brief  ���񴴽�����������������ڴ˴����� 
  * @param  ��
  * @retval ��
  ***************************************/
void Task_init(void)
{

#ifdef Creat_Wav_Player
    /*���ֲ���������*/
    Wav_Player = rt_thread_create("Wav_Player_Task", Wav_Player_Task, RT_NULL, 2048, 1, 100);
    if (Wav_Player != RT_NULL)
        rt_thread_startup(Wav_Player);
#endif
#ifdef Creat_USB_Transfer
    /*USB�������洢����*/
    USB_Transfer = rt_thread_create("USB_Transfer_Task", USB_Transfer_Task, RT_NULL, 512, 0, 20);
    if (USB_Transfer != RT_NULL)
        rt_thread_startup(USB_Transfer);
#endif
#ifdef Creat_OLED_Display
    /*OLED��ʾ����*/
    OLED_Display = rt_thread_create("OLED_Display_Task", OLED_Display_Task, RT_NULL, 512, 1, 20);
    if (OLED_Display != RT_NULL)
        rt_thread_startup(OLED_Display);
#endif
#ifdef Creat_NFC_Transfer
    /*NFC����*/
    NFC_Transfer = rt_thread_create("NFC_Transfer_Task", NFC_Transfer_Task, RT_NULL, 1024, 1, 20);
    if (NFC_Transfer != RT_NULL)
        rt_thread_startup(NFC_Transfer);
#endif
#ifdef Creat_Dispose
    Dispose = rt_thread_create("Status_Reflash", Dispose_Task, RT_NULL, 1024, 2, 20);
    if (Dispose != RT_NULL)
        rt_thread_startup(Dispose);
#endif
}
/****************************************
  * @brief  �ź���������������ֵ������ 
  * @param  ��
  * @retval ��
  ***************************************/
void Semaphore_init(void)
{
    USBorAudioUsingSDIO_Mutex = rt_mutex_create("USBorAudioUsingSDIO_Mutex", RT_IPC_FLAG_PRIO);
}
/****************************************
	* @brief  ���䴴������  
  * @param  ��
  * @retval ��
  ***************************************/
void Mailbox_init(void)
{
    NFCTag_CustomID_mb = rt_mb_create("NFCTag_CustomID_mb", 1, RT_IPC_FLAG_FIFO);
    NFC_TagID_mb = rt_mb_create("NFC_TagID_mb", 1, RT_IPC_FLAG_FIFO);
    BuleTooth_Transfer_mb = rt_mb_create("BuleTooth_Transfer_mb", 1, RT_IPC_FLAG_FIFO);
    NFC_SendMAC_mb = rt_mb_create("NFC_SendMAC_mb", 1, RT_IPC_FLAG_FIFO);
    The_Auido_Name_mb = rt_mb_create("The_Auido_Name_mb", 1, RT_IPC_FLAG_FIFO);
    //	 Loop_PlayBack_mb 		 = rt_mb_create("Loop_PlayBack_mb",			1,	RT_IPC_FLAG_FIFO);
    LOW_PWR_mb = rt_mb_create("LOW_PWR_mb", 1, RT_IPC_FLAG_FIFO);
    // 	 NO_Audio_File_mb 		 = rt_mb_create("NO_Audio_File_mb",			1,	RT_IPC_FLAG_FIFO);
}
/****************************************
  * @brief  �¼��������� 
  * @param  ��
  * @retval ��
  ***************************************/
void Event_init(void)
{
    AbortWavplay_Event = rt_event_create("AbortWavplay_Event", RT_IPC_FLAG_FIFO);
    PlayWavplay_Event = rt_event_create("PlaytWavplay_Event", RT_IPC_FLAG_FIFO);
    OLED_Display_Event = rt_event_create("OLED_Display_Event", RT_IPC_FLAG_FIFO);
}
