#include "board.h"
#include "rtthread.h"


/***********************函数声明区*******************************/
void Application(void* parameter);
/***********************声明返回区*******************************/

/***********************全局变量区*******************************/
static rt_thread_t App_Handle = RT_NULL;


int main(void)
{
	App_Handle = rt_thread_create( "Application",              
                      Application,   			 /* 线程入口函数 */
                      RT_NULL,             /* 线程入口函数参数 */
                      512,                 /* 线程栈大小 */
                      1,                   /* 线程的优先级 */
                      20);                 /* 线程时间片 */
                   
    /* 启动线程，开启调度 */
   if (App_Handle != RT_NULL)    rt_thread_startup(App_Handle);
}

void Application(void* parameter)
{
	Semaphore_init();
	Task_init();
	rt_thread_delete(App_Handle);//删除入口函数 
}









