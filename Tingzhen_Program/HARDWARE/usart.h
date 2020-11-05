#ifndef _USART_H
#define _USART_H
#include "sys.h"
#include "stdio.h"	

#define USART_REC_LEN  			255  	//�����������ֽ��� 200
#define EN_USART1_RX 				1		  //ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	
extern UART_HandleTypeDef UART1_Handler; //UART���
extern UART_HandleTypeDef UART3_Handler; //UART���

#define RXBUFFERSIZE   1 //�����С
extern u8 aRxBuffer[RXBUFFERSIZE];//HAL��USART����Buffer

void uart_init(void);
uint8_t Compare_string(const char *file_name,const char *str_name);
void Buff_Clear(uint8_t buf[]);


#endif
