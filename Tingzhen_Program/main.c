#include "board.h"
#include "rtthread.h"

/***********************函数声明区*******************************/
void Application(void* parameter);
void ALL_Init(void);
/***********************声明返回区*******************************/

/***********************全局变量区*******************************/
static rt_thread_t App_Handle = RT_NULL;


int main(void)
{
//	App_Handle = rt_thread_create( "Application",              
//                      Application,   			 /* 线程入口函数 */
//                      RT_NULL,             /* 线程入口函数参数 */
//                      2048,                 /* 线程栈大小 */
//                      1,                   /* 线程的优先级 */
//                      20);                 /* 线程时间片 */
//                   
//    /* 启动线程，开启调度 */
//   if (App_Handle != RT_NULL)    rt_thread_startup(App_Handle);
	ALL_Init();
	Semaphore_init();
	Task_init();
//	rt_thread_delete(App_Handle);//删除入口函数 
}

//void Application(void* parameter)
//{	

//}

void ALL_Init(void)
{
		ENABLE_ALL_SWITCH();
		rt_thread_delay(100);  //延时100ms为了让各个模块初始化完成
		W25QXX_Init();				    //初始化W25Q256
		SPI3_Init();
		WM8978_Init();				    //初始化WM8978
		OLED_Init();
		WM8978_SPKvol_Set(100);		    //喇叭音量设置
		Movie_Show_Img(32,0,0);
    my_mem_init(SRAMIN);            //初始化内部内存池
    my_mem_init(SRAMCCM);           //初始化内部CCM内存池
    exfuns_init();		            //为fatfs相关变量申请内存  
    f_mount(fs[0],"0:",1);          //挂载SD卡 
		f_mount(fs[1],"1:",1);          //挂载SPI FLASH. 		     
		if(font_init())		
			update_font("0:");
		rt_thread_delay(2000);  //延时两秒为了让图片显示出来	
		printf("done\n");
}







