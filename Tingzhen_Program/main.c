#include "board.h"
#include "rtthread.h"

/***********************函数声明区*******************************/
void Application(void* parameter);
void ALL_Init(void);
void PowerOn_Display(void);
/***********************声明返回区*******************************/

/***********************全局变量区*******************************/
static rt_thread_t App_Handle = RT_NULL;

int main(void)
{
	//delay_ms(3000); //若无开机电路，单纯上电开机，系统无法运行，需加此延时
	//PowerOn_Display();
	ALL_Init();
	Event_init();
	Mailbox_init();
	Semaphore_init();
	Task_init();
}
void ALL_Init(void)
{
		OLED_Clear();
		ENABLE_ALL_SWITCH();
		delay_ms(100);  //延时100ms为了让各个模块初始化完成
		W25QXX_Init();				    //初始化W25Q256
		SPI3_Init();
		WM8978_Init();				    //初始化WM8978
		WM8978_HPvol_Set(20,20);

		Movie_Show_Img(32,0,0);
    exfuns_init();		            //为fatfs相关变量申请内存  
    f_mount(fs[0],"0:",1);          //挂载SD卡 
		f_mount(fs[1],"1:",1);          //挂载SPI FLASH. 		     
		if(font_init())		
		{
			OLED_Clear();
			Show_String(48,36,(uint8_t*)"Updating...");	
			update_font("0:");
		}
		delay_ms(2000);  //延时两秒为了让图片显示出来	
	  IWDG_Init(IWDG_PRESCALER_64,3000);//3s
		printf("done\n");
}
void PowerOn_Display(void)
{
	uint8_t Clear_Flag=0;//拔掉充电器，清除屏，只清一次
	DISABLE_ALL_SWITCH();
	delay_ms(3000);//相当于消抖吧
	while((HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) != GPIO_PIN_RESET))
	{	
		//关机充电显示
		if(HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) == GPIO_PIN_RESET)
		{
			ChargeDisplay();	
			Clear_Flag = 1;	
		}
		//这样是为了防止连续向OLED写东西
		if((HAL_GPIO_ReadPin(GPIOC,USB_Connect_Check_PIN) != GPIO_PIN_RESET) && Clear_Flag == 1)
		{
			OLED_Clear();
			Clear_Flag = 0;
		}
		//慢点开机，别一轻点就开机
		delay_ms(2000);
	}
}






