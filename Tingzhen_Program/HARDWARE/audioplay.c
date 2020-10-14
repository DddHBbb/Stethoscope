#include "audioplay.h"
#include "ff.h"
#include "malloc.h"
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "exfuns.h"  
#include "text.h"
#include "string.h"  

 

//���ֲ��ſ�����
__audiodev audiodev;	  
 

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
//�õ�path·����,Ŀ���ļ����ܸ���
//path:·��		    
//����ֵ:����Ч�ļ���
u16 audio_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//��ʱĿ¼
	FILINFO* tfileinfo;	//��ʱ�ļ���Ϣ	 	
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//�����ڴ�
    res=f_opendir(&tdir,(const TCHAR*)path); //��Ŀ¼ 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//��ѯ�ܵ���Ч�ļ���
		{
	        res=f_readdir(&tdir,tfileinfo);       			//��ȡĿ¼�µ�һ���ļ�
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;	//������/��ĩβ��,�˳�	 		 
			res=f_typetell((u8*)tfileinfo->fname);	
			if((res&0XF0)==0X40)//ȡ����λ,�����ǲ��������ļ�	
			{
				rval++;//��Ч�ļ�������1
			}	    
		}  
	}  
	myfree(SRAMIN,tfileinfo);//�ͷ��ڴ�
	return rval;
}

//��������
void audio_play(void)
{
 	DIR wavdir;	 		//Ŀ¼
	FILINFO *wavfileinfo;//�ļ���Ϣ 
	u8 *pname;			//��·�����ļ���

	WM8978_ADDA_Cfg(1,0);	//����DAC
	WM8978_Input_Cfg(0,0,0);//�ر�����ͨ��
	WM8978_Output_Cfg(1,0);	//����DAC���    	
	
	//audio_get_tnum("0:/MUSIC"); //�õ�����Ч�ļ���
	wavfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));	//�����ڴ�
  pname=mymalloc(SRAMIN,_MAX_LFN*2+1);					//Ϊ��·�����ļ��������ڴ�
  f_opendir(&wavdir,(const TCHAR*)"0:/MUSIC"); 	//��Ŀ¼
	f_readdir(&wavdir,wavfileinfo);       				//��ȡĿ¼�µ�һ���ļ�
	 
	strcpy((char*)pname,"0:/MUSIC/");						//����·��(Ŀ¼)
	strcat((char*)pname,(const char*)wavfileinfo->fname);	//���ļ������ں���
	wav_play_song(pname); 			 		//���������Ƶ�ļ�
											   		    
	myfree(SRAMIN,wavfileinfo);			//�ͷ��ڴ�			    
	myfree(SRAMIN,pname);				//�ͷ��ڴ�			    

} 






























