#include "wavplay.h" 
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "exfuns.h"  
#include "text.h"
#include "string.h"  
#include "rtthread.h"
/***********************����������*******************************/
uint8_t Compare_string(const char *file_name,const char *str_name);
/***********************����������*******************************/
extern rt_mailbox_t AbortWavplay_Event;

/***********************ȫ�ֱ�����*******************************/
uint8_t AbortWavplay_Event_Flag=0;

__audiodev audiodev;	//���ֲ��ſ�����
__wavctrl wavctrl;		//WAV���ƽṹ��
vu8 wavtransferend=0;	//sai������ɱ�־
vu8 wavwitchbuf=0;		//saibufxָʾ��־
/****************************************************************/
const char UID_Num[][8]={"B6C89A2B","B6A49C2B","D6E69C2B","C6EC9C2B","964E9D2B"};
const char Song_Name[][20]={"������Ԫǰ.wav","FeelSoClose.wav","Lucky.wav","Shots.wav","viva.wav"}; 

//��ʼ��Ƶ����
void audio_start(void)
{
	audiodev.status=3<<0;//��ʼ����+����ͣ
	SAI_Play_Start();
} 
//�ر���Ƶ����
void audio_stop(void)
{
	audiodev.status=0;
	SAI_Play_Stop();
}  

//��������
void audio_play(char *file_name)
{
	u8 *pname;			//��·�����ļ���
	WM8978_ADDA_Cfg(1,0);	//����DAC
	WM8978_Input_Cfg(0,0,0);//�ر�����ͨ��
	WM8978_Output_Cfg(1,0);	//����DAC���    	

	pname = rt_malloc(_MAX_LFN*2+1); 
	strcpy((char*)pname,"0:/SYSTEM/MUSIC/");						//����·��(Ŀ¼)
	strcat((char*)pname,(const char*)file_name);	//���ļ������ں���
	wav_play_song(pname); 			 		//���������Ƶ�ļ�			
	rt_free(pname);
} 

//WAV������ʼ��
//fname:�ļ�·��+�ļ���
//wavx:wav ��Ϣ��Žṹ��ָ��
//����ֵ:0,�ɹ�;1,���ļ�ʧ��;2,��WAV�ļ�;3,DATA����δ�ҵ�.
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
	ftemp=(FIL*)rt_malloc(sizeof(FIL));
	buf=rt_malloc(512);
	if(ftemp&&buf)	//�ڴ�����ɹ�
	{
		res=f_open(ftemp,(TCHAR*)fname,FA_READ);//���ļ�
		if(res==FR_OK)
		{
			f_read(ftemp,buf,512,&br);	//��ȡ512�ֽ�������
			riff=(ChunkRIFF *)buf;		//��ȡRIFF��
			if(riff->Format==0X45564157)//��WAV�ļ�
			{
				fmt=(ChunkFMT *)(buf+12);	//��ȡFMT�� 
				fact=(ChunkFACT *)(buf+12+8+fmt->ChunkSize);//��ȡFACT��
				if(fact->ChunkID==0X74636166||fact->ChunkID==0X5453494C)
					wavx->datastart=12+8+fmt->ChunkSize+8+fact->ChunkSize;//����fact/LIST���ʱ��(δ����)
				else wavx->datastart=12+8+fmt->ChunkSize;  
				data=(ChunkDATA *)(buf+wavx->datastart);	//��ȡDATA��
				if(data->ChunkID==0X61746164)//�����ɹ�!
				{
					wavx->audioformat=fmt->AudioFormat;		//��Ƶ��ʽ
					wavx->nchannels=fmt->NumOfChannels;		//ͨ����
					wavx->samplerate=fmt->SampleRate;		//������
					wavx->bitrate=fmt->ByteRate*8;			//�õ�λ��
					wavx->blockalign=fmt->BlockAlign;		//�����
					wavx->bps=fmt->BitsPerSample;			//λ��,16/24/32λ
					
					wavx->datasize=data->ChunkSize;			//���ݿ��С
					wavx->datastart=wavx->datastart+8;		//��������ʼ�ĵط�. 
				}else res=3;//data����δ�ҵ�.
			}else res=2;//��wav�ļ�
			
		}else res=1;//���ļ�����
	}
	f_close(ftemp);
	rt_free(ftemp);//�ͷ��ڴ�
	rt_free(buf); 
	return 0;
}

//���buf
//buf:������
//size:���������
//bits:λ��(16/24)
//����ֵ:���������ݸ���
u32 wav_buffill(u8 *buf,u16 size,u8 bits)
{
	u16 readlen=0;
	u32 bread;
	u16 i;
	u32 *p,*pbuf;
	if(bits==24)//24bit��Ƶ,��Ҫ����һ��
	{
		readlen=(size/4)*3;		//�˴�Ҫ��ȡ���ֽ���
		f_read(audiodev.file,audiodev.tbuf,readlen,(UINT*)&bread);//��ȡ���� 
		pbuf=(u32*)buf;
		for(i=0;i<size/4;i++)
		{  
			p=(u32*)(audiodev.tbuf+i*3);
			pbuf[i]=p[0];  
		} 
		bread=(bread*4)/3;		//����Ĵ�С.
	}else 
	{
		f_read(audiodev.file,buf,size,(UINT*)&bread);//16bit��Ƶ,ֱ�Ӷ�ȡ����  
		if(bread<size)//����������,����0
		{
			for(i=bread;i<size-bread;i++)buf[i]=0; 
		}
	}
	return bread;
}  
//WAV����ʱ,SAI DMA����ص�����
void wav_sai_dma_tx_callback(void) 
{   
	u16 i;
	if(DMA2_Stream3->CR&(1<<19))
	{
		wavwitchbuf=0;
		if((audiodev.status&0X01)==0)
		{
			for(i=0;i<WAV_SAI_TX_DMA_BUFSIZE;i++)//��ͣ
			{
				audiodev.saibuf1[i]=0;//���0
			}
		}
	}else 
	{
		wavwitchbuf=1;
		if((audiodev.status&0X01)==0)
		{
			for(i=0;i<WAV_SAI_TX_DMA_BUFSIZE;i++)//��ͣ
			{
				audiodev.saibuf2[i]=0;//���0
			}
		}
	}
	wavtransferend=1;
} 
//����ĳ��WAV�ļ�
//fname:wav�ļ�·��.
//����:����
u8 wav_play_song(u8* fname)
{
	u8 res,*flag=NULL;  
	u32 fillnum; 
	
	audiodev.file=(FIL*)rt_malloc(sizeof(FIL));
	audiodev.saibuf1=(uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);
	audiodev.saibuf2=(uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);
	audiodev.tbuf=(uint8_t *)rt_malloc(WAV_SAI_TX_DMA_BUFSIZE);
	
	if(audiodev.file&&audiodev.saibuf1&&audiodev.saibuf2&&audiodev.tbuf)
	{ 
		res=wav_decode_init(fname,&wavctrl);//�õ��ļ�����Ϣ
		if(res==0)//�����ļ��ɹ�
		{
			if(wavctrl.bps==16)
			{
				WM8978_I2S_Cfg(2,0);	//�����ֱ�׼,16λ���ݳ���
				SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_16);
				SAIA_SampleRate_Set(wavctrl.samplerate);//���ò�����  
				SAIA_TX_DMA_Init(audiodev.saibuf1,audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE/2,1); //����TX DMA,16λ
			}else if(wavctrl.bps==24)
			{
				WM8978_I2S_Cfg(2,2);	//�����ֱ�׼,24λ���ݳ���
				SAIA_Init(SAI_MODEMASTER_TX,SAI_CLOCKSTROBING_RISINGEDGE,SAI_DATASIZE_24);
				SAIA_SampleRate_Set(wavctrl.samplerate);//���ò�����
				SAIA_TX_DMA_Init(audiodev.saibuf1,audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE/4,2); //����TX DMA,32λ                
			}
			sai_tx_callback=wav_sai_dma_tx_callback;			//�ص�����ָwav_sai_dma_callback 
			audio_stop();
			res=f_open(audiodev.file,(TCHAR*)fname,FA_READ);	//���ļ�
			if(res==0)
			{
				f_lseek(audiodev.file, wavctrl.datastart);		//�����ļ�ͷ
				fillnum=wav_buffill(audiodev.saibuf1,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);
				fillnum=wav_buffill(audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);
				audio_start();  
				//��ѭ��ǣ�����ݴ��䣬����ʹ��RT�������֤������������
				while(AbortWavplay_Event_Flag)
				{ 
					while(wavtransferend==0);//�ȴ�wav�������; 
					wavtransferend=0;
					if(fillnum!=WAV_SAI_TX_DMA_BUFSIZE)//���Ž���
						break;
					if(wavwitchbuf)
						fillnum=wav_buffill(audiodev.saibuf2,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);//���buf2
					else 
						fillnum=wav_buffill(audiodev.saibuf1,WAV_SAI_TX_DMA_BUFSIZE,wavctrl.bps);//���buf1
					
					rt_thread_yield();
				}	
				audio_stop(); 
			}else res=0XFF; 
		}else res=0XFF;
	}else res=0XFF; 
	rt_free(audiodev.tbuf);	//�ͷ��ڴ�
	rt_free(audiodev.saibuf1);//�ͷ��ڴ�
	rt_free(audiodev.saibuf2);//�ͷ��ڴ� 
	rt_free(audiodev.file);	//�ͷ��ڴ� 
	return res;
} 

char* Select_File(char *file_name)
{
	rt_kprintf("filename = %s\n",file_name);
	for(int i=0;i<5;i++)
	{
		if(	Compare_string(file_name,UID_Num[i]))
		{
			rt_kprintf("Song_Name = %s\n",Song_Name[i]);
			return (char *)Song_Name[i];
		}
	}
}

uint8_t Compare_string(const char *file_name,const char *str_name)
{
	if(strlen(file_name) != strlen(file_name) ) return 0;
	for(int i=0;i<strlen(file_name);i++)
	{
		if(file_name[i] != str_name[i])
			return 0;
	}
	return 1;
}







