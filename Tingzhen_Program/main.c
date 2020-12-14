#include "board.h"
#include "rtthread.h"

/***********************����������*******************************/
void Application(void* parameter);
void ALL_Init(void);
void PowerOn_Display(void);
/***********************����������*******************************/

/***********************ȫ�ֱ�����*******************************/

int main(void)
{	
	ENABLE_ALL_SWITCH();
	PowerOn_Display();	
	ALL_Init();
	Timer_Init();
	Event_init();
	Mailbox_init();
	Semaphore_init();
	Task_init();
}
void ALL_Init(void)
{
		OLED_Clear();	
		delay_ms(100);  //��ʱ100msΪ���ø���ģ���ʼ�����
		W25QXX_Init();				    //��ʼ��W25Q128 
		SPI3_Init();
		WM8978_Init();				    //��ʼ��WM8978
		WM8978_HPvol_Set(20,20);
		Movie_Show_Img(32,0,0);
    exfuns_init();		            //Ϊfatfs��ر��������ڴ�  
    f_mount(fs[0],"0:",1);          //����SD�� 	     
		if(font_init())		
		{
			OLED_Clear();
			Show_String(36,36,(uint8_t*)"Updating...");	
			update_font("0:");
		}
		delay_ms(2000);  //��ʱ����Ϊ����ͼƬ��ʾ����	
//	  IWDG_Init(IWDG_PRESCALER_64,(500*8));//2s  �͹����޷��뿴�Ź����ã������ⲿ���Ź�оƬ����
		Key_EXTI_Config();
		printf("done\n");
}
void PowerOn_Display(void)
{
	while(1)
	{					
		if((HAL_GPIO_ReadPin(GPIOE,PBout_Pin) == GPIO_PIN_RESET))
		{
			HAL_GPIO_WritePin(GPIOE,PSHOLD_Pin,GPIO_PIN_SET);
			break;
		}		
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{
			break;
		}		
	}
}






