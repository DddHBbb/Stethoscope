#include "audioplay.h"
#include "ff.h"
#include "malloc.h"
#include "usart.h"
#include "wm8978.h"
#include "sai.h"
#include "exfuns.h"  
#include "text.h"
#include "string.h"  

 

//音乐播放控制器
__audiodev audiodev;	  
 

//开始音频播放
void audio_start(void)
{
	audiodev.status=3<<0;//开始播放+非暂停
	SAI_Play_Start();
} 
//关闭音频播放
void audio_stop(void)
{
	audiodev.status=0;
	SAI_Play_Stop();
}  
//得到path路径下,目标文件的总个数
//path:路径		    
//返回值:总有效文件数
u16 audio_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//临时目录
	FILINFO* tfileinfo;	//临时文件信息	 	
	tfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));//申请内存
    res=f_opendir(&tdir,(const TCHAR*)path); //打开目录 
	if(res==FR_OK&&tfileinfo)
	{
		while(1)//查询总的有效文件数
		{
	        res=f_readdir(&tdir,tfileinfo);       			//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出	 		 
			res=f_typetell((u8*)tfileinfo->fname);	
			if((res&0XF0)==0X40)//取高四位,看看是不是音乐文件	
			{
				rval++;//有效文件数增加1
			}	    
		}  
	}  
	myfree(SRAMIN,tfileinfo);//释放内存
	return rval;
}

//播放音乐
void audio_play(void)
{
 	DIR wavdir;	 		//目录
	FILINFO *wavfileinfo;//文件信息 
	u8 *pname;			//带路径的文件名

	WM8978_ADDA_Cfg(1,0);	//开启DAC
	WM8978_Input_Cfg(0,0,0);//关闭输入通道
	WM8978_Output_Cfg(1,0);	//开启DAC输出    	
	
	//audio_get_tnum("0:/MUSIC"); //得到总有效文件数
	wavfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));	//申请内存
  pname=mymalloc(SRAMIN,_MAX_LFN*2+1);					//为带路径的文件名分配内存
  f_opendir(&wavdir,(const TCHAR*)"0:/MUSIC"); 	//打开目录
	f_readdir(&wavdir,wavfileinfo);       				//读取目录下的一个文件
	 
	strcpy((char*)pname,"0:/MUSIC/");						//复制路径(目录)
	strcat((char*)pname,(const char*)wavfileinfo->fname);	//将文件名接在后面
	wav_play_song(pname); 			 		//播放这个音频文件
											   		    
	myfree(SRAMIN,wavfileinfo);			//释放内存			    
	myfree(SRAMIN,pname);				//释放内存			    

} 






























