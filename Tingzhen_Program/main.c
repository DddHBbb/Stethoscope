#include "board.h"
#include "rtthread.h"

/***********************����������*******************************/
void Application(void* parameter);
void ALL_Init(void);
/***********************����������*******************************/

/***********************ȫ�ֱ�����*******************************/
static rt_thread_t App_Handle = RT_NULL;


int main(void)
{
	
	App_Handle = rt_thread_create( "Application",              
                      Application,   			 /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      2048,                 /* �߳�ջ��С */
                      1,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
   if (App_Handle != RT_NULL)    rt_thread_startup(App_Handle);
}

void Application(void* parameter)
{	
	ALL_Init();
	Semaphore_init();
	Task_init();
	rt_thread_delete(App_Handle);//ɾ����ں��� 
}

void ALL_Init(void)
{
		ENABLE_ALL_SWITCH();
		rt_thread_delay(100);  //100ms
		W25QXX_Init();				    //��ʼ��W25Q256
		WM8978_Init();				    //��ʼ��WM8978
		WM8978_SPKvol_Set(100);		    //������������
    my_mem_init(SRAMIN);            //��ʼ���ڲ��ڴ��
    my_mem_init(SRAMCCM);           //��ʼ���ڲ�CCM�ڴ��
    exfuns_init();		            //Ϊfatfs��ر��������ڴ�  
    f_mount(fs[0],"0:",1);          //����SD�� 
		f_mount(fs[1],"1:",1);          //����SPI FLASH. 		     
		if(font_init())		update_font("0:");
		OLED_Init();
		printf("done");
}







