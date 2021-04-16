#include "wavplay.h"
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "exfuns.h"
#include "text.h"
#include "string.h"
#include "rtthread.h"
#include "oled.h"
#include "key.h"
#include "image.h"
#include "D_delay.h"
#include "GPIOConfig.h"
/***********************函数声明区*******************************/
void Adjust_Volume(void);
/***********************声明返回区*******************************/
extern rt_event_t AbortWavplay_Event;
extern rt_event_t PlayWavplay_Event;
extern DMA_HandleTypeDef SAI1_TXDMA_Handler;
extern rt_mailbox_t Stop_Playing_mb;
/***********************全局变量区*******************************/
__audiodev audiodev;    			//音乐播放控制器
__wavctrl wavctrl;      			//WAV控制结构体
uint8_t Display_Flag 	= 0;
vu8 wavtransferend 		= 0; 		//sai传输完成标志
vu8 wavwitchbuf 			= 0;    //saibufx指示标志
static uint8_t volume = 40;
/****************************************************************/
const char UID_Num[][8] 	 = {"B6C89A2B", "B6A49C2B", "D6E69C2B", "C6EC9C2B", "964E9D2B"};
const char Song_Name[][20] = {"音频一", "音频二", "音频三", "音频四", "音频五"};

//开始音频播放
void audio_start(void)
{
    audiodev.status = 3 << 0; //开始播放+非暂停
    SAI_Play_Start();
}
//关闭音频播放
void audio_stop(void)
{
    audiodev.status = 0;
    SAI_Play_Stop();
}
//播放音乐
void audio_play(char *file_name)
{
    u8 *pname;                 //带路径的文件名
    WM8978_ADDA_Cfg(1, 0);     //开启DAC
    WM8978_Input_Cfg(0, 0, 0); //关闭输入通道
    WM8978_Output_Cfg(1, 0);   //开启DAC输出
    pname = rt_malloc(_MAX_LFN * 2 + 1);
    strcpy((char *)pname, "0:/SYSTEM/MUSIC/"); //复制路径(目录)
    strcat((char *)pname, file_name);
    wav_play_song(pname); //播放这个音频文件
    rt_free(pname);
}

//WAV解析初始化
//fname:文件路径+文件名
//wavx:wav 信息存放结构体指针
//返回值:0,成功;1,打开文件失败;2,非WAV文件;3,DATA区域未找到.
u8 wav_decode_init(u8 *fname, __wavctrl *wavx)
{
    FIL *ftemp;
    u8 *buf;
    u32 br = 0;
    u8 res = 0;

    ChunkRIFF *riff;
    ChunkFMT *fmt;
    ChunkFACT *fact;
    ChunkDATA *data;
    ftemp = (FIL *)rt_malloc(sizeof(FIL));
    buf = rt_malloc(512);
    if (ftemp && buf) //内存申请成功
    {
        res = f_open(ftemp, (TCHAR *)fname, FA_READ); //打开文件
        if (res == FR_OK)
        {
            Show_String(32, 32, (uint8_t *)"正在播放");
            OLED_Refresh_Gram();
            f_read(ftemp, buf, 512, &br);   //读取512字节在数据
            riff = (ChunkRIFF *)buf;        //获取RIFF块
            if (riff->Format == 0X45564157) //是WAV文件
            {
                fmt = (ChunkFMT *)(buf + 12);                        //获取FMT块
                fact = (ChunkFACT *)(buf + 12 + 8 + fmt->ChunkSize); //读取FACT块
                if (fact->ChunkID == 0X74636166 || fact->ChunkID == 0X5453494C)
                    wavx->datastart = 12 + 8 + fmt->ChunkSize + 8 + fact->ChunkSize; //具有fact/LIST块的时候(未测试)
                else
                    wavx->datastart = 12 + 8 + fmt->ChunkSize;
                data = (ChunkDATA *)(buf + wavx->datastart);  //读取DATA块
                if (data->ChunkID == 0X61746164)              //解析成功!
                {
                    wavx->audioformat = fmt->AudioFormat; 		//音频格式
                    wavx->nchannels   = fmt->NumOfChannels; 	//通道数
                    wavx->samplerate  = fmt->SampleRate;   		//采样率  fmt->SampleRate
                    wavx->bitrate     = fmt->ByteRate * 8;    //得到位速
                    wavx->blockalign  = fmt->BlockAlign;   		//块对齐
                    wavx->bps         = fmt->BitsPerSample;   //位数,16/24/32位
                    wavx->datasize    = data->ChunkSize;      //数据块大小
                    wavx->datastart   = wavx->datastart + 8;  //数据流开始的地方.
                }
                else
                    res = 3; //data区域未找到.
            }
            else
                res = 2; //非wav文件
        }
        else
            res = 1; //打开文件错误
    }
    f_close(ftemp);
    rt_free(ftemp); //释放内存
    rt_free(buf);
    return 0;
}

//填充buf
//buf:数据区
//size:填充数据量
//bits:位数(16/24)
//返回值:读到的数据个数
u32 wav_buffill(u8 *buf, u16 size, u8 bits)
{
    u16 readlen = 0;
    u32 bread;
    u16 i;
    u32 *p, *pbuf;
    if (bits == 24) //24bit音频,需要处理一下
    {
        readlen = (size / 4) * 3;                                      //此次要读取的字节数
        f_read(audiodev.file, audiodev.tbuf, readlen, (UINT *)&bread); //读取数据
        pbuf = (u32 *)buf;
        for (i = 0; i < size / 4; i++)
        {
            p = (u32 *)(audiodev.tbuf + i * 3);
            pbuf[i] = p[0];
        }
        bread = (bread * 4) / 3; //填充后的大小.
    }
    else
    {
        f_read(audiodev.file, buf, size, (UINT *)&bread); //16bit音频,直接读取数据
        if (bread < size)                                 //不够数据了,补充0
        {
            for (i = bread; i < size - bread; i++)
                buf[i] = 0;
        }
    }
    return bread;
}
//WAV播放时,SAI DMA传输回调函数
void wav_sai_dma_tx_callback(void)
{
    u16 i;
    if (DMA2_Stream3->CR & (1 << 19))
    {
        wavwitchbuf = 0;
        if ((audiodev.status & 0X01) == 0)
        {
            for (i = 0; i < WAV_SAI_TX_DMA_BUFSIZE; i++) //暂停
            {
                audiodev.saibuf1[i] = 0; //填充0
            }
        }
    }
    else
    {
        wavwitchbuf = 1;
        if ((audiodev.status & 0X01) == 0)
        {
            for (i = 0; i < WAV_SAI_TX_DMA_BUFSIZE; i++) //暂停
            {
                audiodev.saibuf2[i] = 0; //填充0
            }
        }
    }
    wavtransferend = 1;
}
//播放某个WAV文件
//fname:wav文件路径.
//其他:错误
u8 wav_play_song(u8 *fname)
{
    u8 res;
    uint8_t t = 0;
    u32 fillnum = 0;
    static rt_uint32_t Play_rev = 0;
    static rt_uint32_t Abort_rev = 0;
    uint32_t timeout = 0;
    uint32_t maxDelay = 0xFFFFF;
    uint8_t TimeOut_FLAG = 0;

    audiodev.file = (FIL *)rt_malloc(sizeof(FIL));
    audiodev.saibuf1 = (uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);
    audiodev.saibuf2 = (uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);
    audiodev.tbuf = (uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);

    if (audiodev.file && audiodev.saibuf1 && audiodev.saibuf2 && audiodev.tbuf)
    {
        res = wav_decode_init(fname, &wavctrl); //得到文件的信息
        if (res == 0)                           //解析文件成功
        {
            if (wavctrl.bps == 16)
            {
                WM8978_I2S_Cfg(2, 0); //飞利浦标准,16位数据长度
                SAIA_Init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_16);
                SAIA_SampleRate_Set(wavctrl.samplerate);                                             //设置采样率
                SAIA_TX_DMA_Init(audiodev.saibuf1, audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 2, 1); //配置TX DMA,16位
            }
            else if (wavctrl.bps == 24)
            {
                WM8978_I2S_Cfg(2, 2); //飞利浦标准,24位数据长度
                SAIA_Init(SAI_MODEMASTER_TX, SAI_CLOCKSTROBING_RISINGEDGE, SAI_DATASIZE_24);
                SAIA_SampleRate_Set(wavctrl.samplerate);                                             //设置采样率
                SAIA_TX_DMA_Init(audiodev.saibuf1, audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE / 4, 2); //配置TX DMA,32位
            }
            sai_tx_callback = wav_sai_dma_tx_callback; //回调函数指wav_sai_dma_callback
            audio_stop();
            res = f_open(audiodev.file, (TCHAR *)fname, FA_READ); //打开文件
            if (res == 0)
            {
                f_lseek(audiodev.file, wavctrl.datastart); //跳过文件头
                fillnum = wav_buffill(audiodev.saibuf1, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                fillnum = wav_buffill(audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                audio_start();
                Display_Flag = 1; //正在播放
                /*此循环牵扯数据传输，不宜使用RT组件，保证其数据完整性
									由于系统调度，此死循环不一定为死循环，可能导致播放不完整*/
                while (1)
                {
                    //上电总会意外的播放，加入此停止事件可修复
                    rt_event_recv(AbortWavplay_Event, 1 | 2, RT_EVENT_FLAG_OR, RT_WAITING_NO, &Abort_rev); //几us
                    rt_event_recv(PlayWavplay_Event, 1, RT_EVENT_FLAG_OR, RT_WAITING_NO, &Play_rev);       //几us
                    Adjust_Volume();
									
									if (HAL_GPIO_ReadPin(GPIOC, USB_Connect_Check_PIN) == GPIO_PIN_RESET)//检测到USB直接跳出
											break;		
									if ((rt_mb_recv(Stop_Playing_mb, NULL, RT_WAITING_NO)) == RT_EOK)//收到停止播放的命令
											break;
                    if (Play_rev == 1)
                    {
                        while (wavtransferend == 0) //等待wav传输完成;
                        {
                            if ((SAI1_TXDMA_Handler.ErrorCode != HAL_DMA_ERROR_NONE) || (SAI1_TXDMA_Handler.State != HAL_DMA_STATE_BUSY_MEM0))
                            {
                                rt_kprintf("SAI_ERROR=%d\n\r", SAI1_TXDMA_Handler.ErrorCode);
                                rt_kprintf("SAI_STA=%d\n\r", SAI1_TXDMA_Handler.State);
                                SAI1_TXDMA_Handler.ErrorCode = HAL_DMA_ERROR_NONE;
                                SAI1_TXDMA_Handler.State = HAL_DMA_STATE_READY_MEM0;
                                break;
                            }
                            timeout++; //超时处理
                            if (timeout > maxDelay)
                            {
                                TimeOut_FLAG = 1;
                                wav_sai_dma_tx_callback(); //发生错误，复位各项参数
                                rt_kprintf("卡死这里\n\r");
                                break;
                            }
                        }
                        timeout = 0;
                        if (TimeOut_FLAG == 1) //发生错误，直接跳出
                            break;
                        wavtransferend = 0;
                        if (fillnum != WAV_SAI_TX_DMA_BUFSIZE) //播放结束
                        {
														audio_stop();											
                            rt_kprintf("播放完%d遍\n\r", ++t);
                            rt_thread_delay(1000);
														audio_start();
                            f_open(audiodev.file, (TCHAR *)fname, FA_READ);
                            f_lseek(audiodev.file, wavctrl.datastart); //跳过文件头
                            fillnum = wav_buffill(audiodev.saibuf1, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                            fillnum = wav_buffill(audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps);
                        }
                        if (wavwitchbuf)
                            fillnum = wav_buffill(audiodev.saibuf2, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps); //填充buf2
                        else
                            fillnum = wav_buffill(audiodev.saibuf1, WAV_SAI_TX_DMA_BUFSIZE, wavctrl.bps); //填充buf1
                        wavtransferend = 0;
                    }
                    if (Abort_rev == 1) //1为停止播放，2|1为不停止
                        break;
                    //状态清0
                    rt_event_control(AbortWavplay_Event, RT_IPC_CMD_RESET, 0);
                    rt_thread_delay(30);
                }
                Display_Flag = 0; //停止播放
                rt_event_control(AbortWavplay_Event, RT_IPC_CMD_RESET, 0);
                rt_event_control(PlayWavplay_Event, RT_IPC_CMD_RESET, 0);
                audio_stop();
            }
            else
                res = 0XFF;
        }
        else
            res = 0XFF;
    }
    else
        res = 0XFF;
    f_close(audiodev.file);    //关闭文件
    rt_free(audiodev.tbuf);    //释放内存
    rt_free(audiodev.saibuf1); //释放内存
    rt_free(audiodev.saibuf2); //释放内存
    rt_free(audiodev.file);    //释放内存
    return res;
}

char *Select_File(char *file_name)
{
    //	rt_kprintf("filename = %s\n",file_name);
    for (int i = 0; i < 5; i++)
    {
        if (Compare_string(file_name, UID_Num[i]))
        {
            //		rt_kprintf("Song_Name = %s\n",Song_Name[i]);
            return (char *)Song_Name[i];
        }
    }
		return 0;
}

uint8_t Compare_string(const char *file_name, const char *str_name)
{
    if (strlen(file_name) != strlen(str_name))
        return 0;
    for (int i = 0; i < strlen(file_name); i++)
    {
        if (file_name[i] != str_name[i])
            return 0;
    }
    return 1;
}

void Adjust_Volume(void)
{
    uint8_t key = 0;

    key = KEY_Scan();
    while (key !=0 && key != 4)
    {
        OLED_Clear();
        if (key == KEY_UP)
        {
            volume += 10;
            if (volume > 50)
                volume = 50;
        }
        else if (key == KEY_DOWN)
        {
            if (volume == 0)
                volume = 0;
            else
                volume -= 10;
        }
        OLED_ShowNum(0, 0, volume * 2, 3, 16, 1);
        OLED_ShowChar(30, 0, '%', 16, 1);
        VolumeShow(40, 0, 48, 48, 0,  gImage_volume_3per);
        OLED_Fill(16,  50, 22, 	62, 	volume / 10);
        OLED_Fill(26,  50, 32, 	62, 	volume / 10);
        OLED_Fill(36,  50, 42, 	62, 	volume / 20);
        OLED_Fill(46,  50, 52, 	62, 	volume / 20);
        OLED_Fill(56,  50, 62, 	62, 	volume / 30);
        OLED_Fill(66,  50, 72, 	62, 	volume / 30);
        OLED_Fill(76,  50, 82, 	62, 	volume / 40);
        OLED_Fill(86,  50, 92, 	62, 	volume / 40);
        OLED_Fill(96,  50, 102, 62,   volume / 50);
        OLED_Fill(106, 50, 112, 62, 	volume / 50);
        WM8978_HPvol_Set(volume, 0);

        BattChek();
        OLED_Refresh_Gram();
        break;
    }
    if (key == KEY_OK)
    {
        /*以下函数是为了从调整音量播放界面调整回正常显示*/
        OLED_Clear();
        Show_String(0, 0, (uint8_t *)"播放状态：");
        if (Display_Flag == 1)
            Show_String(32, 32, (uint8_t *)"正在播放");
        else
            Show_String(32, 32, (uint8_t *)"停止播放");
        BattChek();
        OLED_Refresh_Gram();
    }
}























