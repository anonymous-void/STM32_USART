#ifndef __INTERRUPT_
#define __INTERRUPT_
#include <stm32f10x_rcc.h>
extern u8 gu8_dataTxState; // 数据传输状态，0为停止，1为开始传输

void Interrupt_Configuration(void);
void External_Configuration(void);
#endif
