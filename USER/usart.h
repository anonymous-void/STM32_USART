#include "stm32f10x_rcc.h"
#ifndef __USART_H
#define __USART_H

void USART1_Configuration(void);
void USART2_Configuration(void);

void USART1_send(u8 ch);
void USART2_send(u8 ch);

extern u8 USART_RX_BUF[64];
//接收状态
//bit7  接受完成标志
//bit6  接收到0x0d \r
//bit5  接收到的有效字节数
extern u8 USART_RX_STA;
#endif
