#include "board.h"
#include "rtthread.h"

/***********************函数声明区*******************************/
void Application(void* parameter);
void ALL_Init(void);
void PowerOn_Display(void);
/***********************声明返回区*******************************/

/***********************全局变量区*******************************/

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
		delay_ms(100);  //延时100ms为了让各个模块初始化完成
		W25QXX_Init();				    //初始化W25Q128 
		SPI3_Init();
		WM8978_Init();				    //初始化WM8978
		WM8978_HPvol_Set(20,20);
		Movie_Show_Img(32,0,0);
    exfuns_init();		            //为fatfs相关变量申请内存  
    f_mount(fs[0],"0:",1);          //挂载SD卡 	     
		if(font_init())		
		{
			OLED_Clear();
			Show_String(36,36,(uint8_t*)"Updating...");	
			update_font("0:");
		}
		delay_ms(2000);  //延时两秒为了让图片显示出来	
//	  IWDG_Init(IWDG_PRESCALER_64,(500*8));//2s  低功耗无法与看门狗共用，可用外部看门狗芯片代替
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






