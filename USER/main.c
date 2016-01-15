
#include "stm32f10x_rcc.h"
#include <stdio.h>
//#include "stm32f10x_gpio.h"

USART_InitTypeDef USART_InitStructure;


void RCC_Configuration(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); // Wait for External Clock stablize.

    RCC_HCLKConfig(RCC_SYSCLK_Div1); // Advance High Speed Bus AHB prescaler = /1
    RCC_PCLK2Config(RCC_HCLK_Div1); // High-speed peripheral APB2 prescaler = /1
    RCC_PCLK1Config(RCC_HCLK_Div2); // Low-speed peripheral APB1 prescaler = /2
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 
    
    RCC_PLLCmd( ENABLE );
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
    
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while ( RCC_GetSYSCLKSource() != 0x08 );
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // GPIO-A and GPIO-B clock Enabled
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // USART-1 Enabled
    
}



void USART_Configuration(void)
{

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity =  USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);
}

void GPIO_Configuration(void)
{
    //CLK:PB5  CLR:PE11  Data:PE10
    GPIO_InitTypeDef        GPIO_InitStructure;                //声明一个结构体变量

    //配置USART1_Tx为复合推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //配置 USART1_Rx 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


void Delay_MS(u16 dly)
{
        u16 i,j;
        for(i=0;i<dly;i++)
        for(j=1000;j>0;j--);
}
void delay_us(u16 dly1)
{
        u16 i;
        for(i=dly1;i>0;i--);
}


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

return ch; 
} 
#endif 
//加入以上代码,支持printf函数,而不需要选择use MicroLIB 

void Interrupt_Configuration()
{
    SCB->AIRCR |= 0xFA05
}

int main()
{
        #ifdef DEBUG
        debug();
        #endif

        int i = 0;

        //------------初始化------------
        RCC_Configuration();
        GPIO_Configuration();
        USART_Configuration( );
        while(1)
        {
          Delay_MS(1000);
          if ( i >= 100 )
              i = 0;
          i ++;
          
          GPIO_SetBits(GPIOA, GPIO_Pin_8);
          Delay_MS(1000);
          printf("%d\r\n", i);
          GPIO_ResetBits(GPIOA, GPIO_Pin_8);
          Delay_MS(1000);


        }
}

