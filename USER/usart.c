#include "usart.h"
#include "stm32f10x_rcc.h"
#include <stdio.h>

u8 USART_RX_BUF[64];
//接收状态
//bit7  接受完成标志
//bit6  接收到0x0d \r
//bit5  接收到的有效字节数
u8 USART_RX_STA=0;

//加入以下代码,支持printf函数,而不需要选择use MicroLIB 
#if 1 
#pragma import(__use_no_semihosting)              
//标准库需要的支持函数                  
struct __FILE  
{  
int handle;  

};  

FILE __stdout;        
//定义_sys_exit()以避免使用半主机模式     
_sys_exit(int x)  
{  
x = x;  
}  
//重定义fputc函数  
int fputc(int ch, FILE *f) 
{       

while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
        USART1->DR = (u8) ch;
while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
        USART2->DR = (u8) ch;

return ch; 
} 
#endif 
//加入以上代码,支持printf函数,而不需要选择use MicroLIB 

void USART2_send(u8 ch)
{
    while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
    USART2->DR = (u8) ch;
}

void USART1_send(u8 ch)
{
    while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;
}

void USART1_Configuration(void)
{
    USART_InitTypeDef  USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity =  USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void USART2_Configuration(void)
{
    USART_InitTypeDef  USART_InitStructure;
    
    USART_DeInit(USART2);
    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
}


void USART1_IRQHandler(void)
{
// \n = 0x0a  \r = 0x0d
//接收状态
//bit7  接受完成标志
//bit6  接收到0x0d \r
//bit5  接收到的有效字节数
    u8 res;
    if(USART1->SR&(1<<5))
    {
        res=USART1->DR;
        if((USART_RX_STA&0x80)==0) // 如果bit7 != 0，接收未完成
        {
            if(USART_RX_STA&0x40) // 如果bit6 == 1，那么表明接收到了\r
            {
                if(res != 0x0a) // 如果之前已经收到\r，而下一个字符不是\n，则传输错误，重新开始接收。
                    USART_RX_STA = 0;
                else
                    USART_RX_STA |= 0x80; // 如果连续收到0x0d 0x0a（\r\n），则接收完成，标志位置位。
            }
            else
            {
                if(res==0x0d)
                    USART_RX_STA|=0x40;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3F]=res;
                    USART_RX_STA++;
                    if(USART_RX_STA>63)
                        USART_RX_STA=0;
                }
            }
        }
    }
}

void USART2_IRQHandler(void)
{
// \n = 0x0a  \r = 0x0d
//接收状态
//bit7  接受完成标志
//bit6  接收到0x0d \r
//bit5  接收到的有效字节数
    u8 res;
    if(USART2->SR&(1<<5))
    {
        res=USART2->DR;
        if((USART_RX_STA&0x80)==0) // 如果bit7 != 0，接收未完成
        {
            if(USART_RX_STA&0x40) // 如果bit6 == 1，那么表明接收到了\r
            {
                if(res != 0x0a) // 如果之前已经收到\r，而下一个字符不是\n，则传输错误，重新开始接收。
                    USART_RX_STA = 0;
                else
                    USART_RX_STA |= 0x80; // 如果连续收到0x0d 0x0a（\r\n），则接收完成，标志位置位。
            }
            else
            {
                if(res==0x0d)
                    USART_RX_STA|=0x40;
                else
                {
                    USART_RX_BUF[USART_RX_STA&0X3F]=res;
                    USART_RX_STA++;
                    if(USART_RX_STA>63)
                        USART_RX_STA=0;
                }
            }
        }
    }
}

