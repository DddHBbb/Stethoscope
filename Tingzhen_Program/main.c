#include "board.h"
#include "rtthread.h"


/***********************����������*******************************/
void Application(void* parameter);
/***********************����������*******************************/

/***********************ȫ�ֱ�����*******************************/
static rt_thread_t App_Handle = RT_NULL;


int main(void)
{
	App_Handle = rt_thread_create( "Application",              
                      Application,   			 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      512,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (App_Handle != RT_NULL)    rt_thread_startup(App_Handle);
}

void Application(void* parameter)
{
	Semaphore_init();
	Task_init();
	rt_thread_delete(App_Handle);//ɾ����ں��� 
}









