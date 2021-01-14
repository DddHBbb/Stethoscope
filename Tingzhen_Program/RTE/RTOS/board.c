/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "board.h" 

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);
extern UART_HandleTypeDef UART1_Handler; //UART句柄
// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE    1024*40
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    /* System Clock Update */
	 // SCB->VTOR = FLASH_BASE | 0x10000;//设置偏移量
    SystemCoreClockUpdate();
		HAL_Init();                     //初始化HAL库   
    Stm32_Clock_Init(384,25,2,8);   //设置时钟,180Mhz
		delay_init(192);
		ALL_GPIO_init();	
		uart_init();  						//初始化USART
		Key_GPIO_Config();
		OLED_Init();
		
	   /* System Tick Configuration */

   _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}


void rt_hw_console_output(const char *str)
{		
	rt_size_t i = 0, size = 0;
	char a = '\r';
	
	__HAL_UNLOCK(&UART1_Handler);
	size = rt_strlen(str);
	for (i = 0; i < size; i++)
	{
		if (*(str + i) == '\n')
		{
				HAL_UART_Transmit(&UART1_Handler, (uint8_t *)&a, 1, 1);
		}
		HAL_UART_Transmit(&UART1_Handler, (uint8_t *)(str + i), 1, 1);
	}			
}
char rt_hw_console_getchar(void)
{
	int ch = -1;
	if (__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_RXNE) != RESET)
	{
			ch = UART1_Handler.Instance->DR & 0xff;
	}
	else
	{
			if(__HAL_UART_GET_FLAG(&UART1_Handler, UART_FLAG_ORE) != RESET)
			{
					__HAL_UART_CLEAR_OREFLAG(&UART1_Handler);
			}
			rt_thread_mdelay(10);
	}
    return ch;
}
void SysTick_Handler(void)
{
		
    /* 进入中断 */
    rt_interrupt_enter();
		HAL_IncTick();
    /* 更新时基 */
    rt_tick_increase();

    /* 离开中断 */
    rt_interrupt_leave();
}
uint32_t HAL_GetTick(void)
{
    return rt_tick_get() * 1000 / RT_TICK_PER_SECOND;
}

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = SystemCoreClock / 1000000UL;
    do {
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    } while(delta < us_tick * us);
}
















