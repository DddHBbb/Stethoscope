#include "board.h"
#include "rtthread.h"
 
/***********************����������*******************************/
void Application(void *parameter);
void ALL_Init(void);
void PowerOn_Display(void);
/***********************����������*******************************/

/***********************ȫ�ֱ�����*******************************/

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
    delay_ms(100); //��ʱ100msΪ���ø���ģ���ʼ�����
    W25QXX_Init(); //��ʼ��W25Q128
    SPI3_Init();
    WM8978_Init(); //��ʼ��WM8978
    WM8978_HPvol_Set(50, 0);
    Movie_Show_Img(32, 0, 0);
    exfuns_init();           //Ϊfatfs��ر��������ڴ�
    f_mount(fs[0], "0:", 1); //����SD��
    if (font_init())
    {
        OLED_Clear();
        Show_String(32, 32, (uint8_t *)"Updating");
				OLED_Refresh_Gram();
        update_font("0:");
    }
    delay_ms(2000); //��ʱ����Ϊ����ͼƬ��ʾ����
    LOWPWR_EXTI_Config();
    //		RTC_Init();
    //		IWDG_Init(IWDG_PRESCALER_64,(500*7));//2s  ��ʱ����ι�����߿����ⲿ���Ź�оƬ����
}
void PowerOn_Display(void)
{
    HAL_GPIO_WritePin(GPIOE, PSHOLD_Pin, GPIO_PIN_SET);
}














