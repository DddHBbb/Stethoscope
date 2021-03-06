#ifndef __OLED_H
#define __OLED_H
#include "sys.h"

		    						  
//-----------------OLED端口定义----------------  		
#define OLED_CS     PDout(4)
#define OLED_RST    PDout(3)
#define OLED_RS     PDout(1)
#define OLED_SCLK   PDout(0)
#define OLED_SDIN   PAout(15)

#define OLED_CMD  	0		//写命令
#define OLED_DATA 	1		//写数据

//OLED控制用函数
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Refresh_Gram(void);		   
							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size,u8 mode);
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size);
void OLED_Show_Font(u16 x,u16 y,u8 fnum);
void Show_String(uint8_t x,uint8_t y,uint8_t *str);
void BattChek(void);
void Movie_Show_Img(uint8_t x,uint8_t y,uint32_t picx);
void DispCurrentPosition(const uint8_t *pbuf);
void BluetoothDisp(u8 t);
void ChargeDisplay(void);
void Battery_Capacity_Transmit(void);
void VolumeShow(uint8_t x,uint8_t y,uint8_t xsize,uint8_t ysize,uint32_t picx,const uint8_t str[]);
#endif


























