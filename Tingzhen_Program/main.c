#include "board.h"
#include "rtthread.h"
 
/***********************函数声明区*******************************/
void Application(void *parameter);
void ALL_Init(void);
void PowerOn_Display(void);
/***********************声明返回区*******************************/

/***********************全局变量区*******************************/

int main(void)
{
		rt_enter_critical();
    ENABLE_ALL_SWITCH();
    PowerOn_Display();
    ALL_Init();
    Timer_Init();
    Event_init();
    Mailbox_init();
    Semaphore_init();
		rt_exit_critical();
    Task_init();
}
void ALL_Init(void)
{	
    OLED_Clear();
    delay_ms(100); //延时100ms为了让各个模块初始化完成
    W25QXX_Init(); //初始化W25Q128
    SPI3_Init();
    WM8978_Init(); //初始化WM8978
    WM8978_HPvol_Set(50, 0);
    Movie_Show_Img(32, 0, 0);
    exfuns_init();           //为fatfs相关变量申请内存
    f_mount(fs[0], "0:", 1); //挂载SD卡
    if (font_init())
    {
        OLED_Clear();
        Show_String(32, 32, (uint8_t *)"Updating");
				OLED_Refresh_Gram();
        update_font("0:");
    }
    delay_ms(2000); //延时两秒为了让图片显示出来
    LOWPWR_EXTI_Config();
    //		RTC_Init();
    //		IWDG_Init(IWDG_PRESCALER_64,(500*7));//2s  定时开启喂狗或者可用外部看门狗芯片代替
}
void PowerOn_Display(void)
{
    HAL_GPIO_WritePin(GPIOE, PSHOLD_Pin, GPIO_PIN_SET);
}














