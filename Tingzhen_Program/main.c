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
	DISABLE_ALL_SWITCH();
	delay_ms(5000);//�൱��������
	while((HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) != GPIO_PIN_RESET))
	{
		delay_ms(2000);//���㿪������һ���Ϳ���
	}
	ALL_Init();
	Event_init();
	Mailbox_init();
	Semaphore_init();
	Task_init();
}

void ALL_Init(void)
{
		ENABLE_ALL_SWITCH();
		delay_ms(100);  //��ʱ100msΪ���ø���ģ���ʼ�����
		W25QXX_Init();				    //��ʼ��W25Q256
		SPI3_Init();
		WM8978_Init();				    //��ʼ��WM8978
		OLED_Init();
		WM8978_SPKvol_Set(100);	    //������������
		WM8978_HPvol_Set(100,100);
		Movie_Show_Img(32,0,0);
    exfuns_init();		            //Ϊfatfs��ر��������ڴ�  
    f_mount(fs[0],"0:",1);          //����SD�� 
		f_mount(fs[1],"1:",1);          //����SPI FLASH. 		     
		if(font_init())		update_font("0:");
		delay_ms(2000);  //��ʱ����Ϊ����ͼƬ��ʾ����	
	  IWDG_Init(IWDG_PRESCALER_64,3000);//3s
		printf("done\n");
}







