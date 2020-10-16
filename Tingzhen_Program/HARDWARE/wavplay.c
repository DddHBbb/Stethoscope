#include "wavplay.h" 
#include "audioplay.h"
#include "usart.h" 
#include "malloc.h"
#include "ff.h"
#include "sai.h"
#include "wm8978.h"
#include "rtthread.h"


 
__wavctrl wavctrl;		//WAV控制结构体
vu8 wavtransferend=0;	//sai传输完成标志
vu8 wavwitchbuf=0;		//saibufx指示标志
 
//WAV解析初始化
//fname:文件路径+文件名
//wavx:wav 信息存放结构体指针
//返回值:0,成功;1,打开文件失败;2,非WAV文件;3,DATA区域未找到.
u8 wav_decode_init(u8* fname,__wavctrl* wavx)
{
	FIL*ftemp;
	u8 *buf; 
	u32 br=0;
	u8 res=0;
	
	ChunkRIFF *riff;
	ChunkFMT *fmt;
	ChunkFACT *fact;
	ChunkDATA *data;
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	buf=mymalloc(SRAMIN,512);
	if(ftemp&&buf)	//内存申请成功
	{
		res=f_open(ftemp,(TCHAR*)fname,FA_READ);//打开文件
		if(res==FR_OK)
		{
			f_read(ftemp,buf,512,&br);	//读取512字节在数据
			riff=(ChunkRIFF *)buf;		//获取RIFF块
			if(riff->Format==0X45564157)//是WAV文件
			{
				fmt=(ChunkFMT *)(buf+12);	//获取FMT块 
				fact=(ChunkFACT *)(buf+12+8+fmt->ChunkSize);//读取FACT块
				if(fact->ChunkID==0X74636166||fact->ChunkID==0X5453494C)
						 wavx->datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;//具有fact/LIST块的时候(未测试)
				else wavx->datastart=12+8+fmt->ChunkSize;  
				data=(ChunkDATA *)(buf+wavx->datastart);	//读取DATA块
				if(data->ChunkID==0X61746164)//解析成功!
				{
					wavx->audioformat=fmt->AudioFormat;		//音频格式
					wavx->nchannels=fmt->NumOfChannels;		//通道数
					wavx->samplerate=8000;		//采样率
					wavx->bitrate=fmt->ByteRate*8;			//得到位速
					wavx->blockalign=fmt->BlockAlign;		//块对齐
					wavx->bps=fmt->BitsPerSample;			//位数,16/24/32位
					
					wavx->datasize=data->ChunkSize;			//数据块大小
					wavx->datastart=wavx->datastart+8;		//数据流开始的地方. 
					 
//					printf("wavx->audioformat:%d\r\n",wavx->audioformat);
//					printf("wavx->nchannels:%d\r\n",wavx->nchannels);
//					printf("wavx->samplerate:%d\r\n",wavx->samplerate);
//					printf("wavx->bitrate:%d\r\n",wavx->bitrate);
//					printf("wavx->blockalign:%d\r\n",wavx->blockalign);
//					printf("wavx->bps:%d\r\n",wavx->bps);
//					printf("wavx->datasize:%d\r\n",wavx->datasize);
//					printf("wavx->datastart:%d\r\n",wavx->datastart);  
				}else res=3;//data区域未找到.
			}else res=2;//非wav文件
			
		}else res=1;//打开文件错误
	}
	f_close(ftemp);
	myfree(SRAMIN,ftemp);//释放内存
	myfree(SRAMIN,buf); 
	return 0;
}

//填充buf
//buf:数据区
//size:填充数据量
//bits:位数(16/24)
//返回值:读到的数据个数
u32 wav_buffill(u8 *buf,u16 size,u8 bits)
{
	u16 readlen=0;
	u32 bread;
	u16 i;
	u32 *p,*pbuf;
	if(bits==24)//24bit音频,需要处理一下
	{
		readlen=(size/4)*3;		//此次要读取的字节数
		f_read(audiodev.file,audiodev.tbuf,readlen,(UINT*)&bread);//读取数据 
		pbuf=(u32*)buf;
		for(i=0;i<size/4;i++)
		{  
			p=(u32*)(audiodev.tbuf+i*3);
			pbuf[i]=p[0];  
		} 
		bread=(bread*4)/3;		//填充后的大小.
	}else 
	{
		f_read(audiodev.file,buf,size,(UINT*)&bread);//16bit音频,直接读取数据  
		if(bread<size)//不够数据了,补充0
		{
			for(i=bread;i<size-bread;i++)buf[i]=0; 
		}
	}
	return bread;
}  
//WAV播放时,SAI DMA传输回调函数
void wav_sai_dma_tx_callback(void) 
{   
	if(DMA2_Stream3->CR&(1<<19))	wavwitchbuf=0;
	else 													wavwitchbuf=1;
	wavtransferend=1;
} 
//播放某个WAV文件
//fname:wav文件路径.
//返回值:
//KEY0_PRES:下一曲
//KEY1_PRES:上一曲
//其他:错误
u8 wav_play_song(u8* fname)
{
	u8 res;  
	u32 fillnum; 
	audiodev.file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));
	audiodev.saibuf1=mymalloc(SRAMIN,WAV_SAI_TX_DMA_BUFSIZE);
	audiodev.saibuf2=mymalloc(SRAMIN,WAV_SAI_TX_DMA_BUFSIZE);
	audiodev.tbuf=mymalloc(SRAMIN,WAV_SAI_TX_DMA_BUFSIZE);
	
	if(audiodev.file&&audiodev.saibuf1&&audiodev.saibuf2&&audiodev.tbuf)
	{ 
		res=wav_decode_init(fname,&wavctrl);//得到文件的信息
		if(res==0)//解析文件成功
		{
			if(wavctrl.bps==16)
			{
				WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度
                SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);
				SAIA_SampleRate_Set(wavctrl.samplerate);//设置采样率  
                SAIA_TX_DMA_Init(audiodev.saibuf1,audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE/2,1); //配置TX DMA,16位
			}else if(wavctrl.bps==24)
			{
				WM8978_I2S_Cfg(2,2);	//飞利浦标准,24位数据长度
                SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_24);
				SAIA_SampleRate_Set(wavctrl.samplerate);//设置采样率
                SAIA_TX_DMA_Init(audiodev.saibuf1,audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE/4,2); //配置TX DMA,32位                
			}
			sai_tx_callback=wav_sai_dma_tx_callback;			//回调函数指wav_sai_dma_callback 
			audio_stop();
			res=f_open(audiodev.file,(TCHAR*)fname,FA_READ);	//打开文件
			if(res==0)
			{
				f_lseek(audiodev.file, wavctrl.datastart);		//跳过文件头
				fillnum=wav_buffill(audiodev.saibuf1,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);
				fillnum=wav_buffill(audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);
				audio_start();  
			
				while(res==0)
				{ 
					while(wavtransferend==0);//等待wav传输完成; 
					wavtransferend=0;
					if(fillnum!=WAV_SAI_TX_DMA_BUFSIZE)//播放结束
					{
						res=0XFF;
						f_close(audiodev.file);
						break;
					} 
 					if(wavwitchbuf)
						fillnum=wav_buffill(audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);//填充buf2
					else 
						fillnum=wav_buffill(audiodev.saibuf1,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);//填充buf1
					while(1)
					{
						if((audiodev.status&0X01)==0)
						{
							for(int i=0;i<100000000;i++);
						}
						else break;
					}
				}
			
				audio_stop(); 
			}else res=0XFF; 
		}else res=0XFF;
	}else res=0XFF; 
	myfree(SRAMIN,audiodev.tbuf);	//释放内存
	myfree(SRAMIN,audiodev.saibuf1);//释放内存
	myfree(SRAMIN,audiodev.saibuf2);//释放内存 
	myfree(SRAMIN,audiodev.file);	//释放内存 
	return res;
} 
	









