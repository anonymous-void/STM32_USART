#include "stm32f10x_rcc.h"
#include <stdio.h>
//#include "stm32f10x_gpio.h"

USART_InitTypeDef USART_InitStructure;

int keyFlag = 0;

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

    // LED Pin Config
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 按键PA0，即WK_UP键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


static u8  fac_us=0;//usÑÓÊ±±¶³ËÊý
static u16 fac_ms=0;//msÑÓÊ±±¶³ËÊý
//³õÊ¼»¯ÑÓ³Ùº¯Êý
//SYSTICKµÄÊ±ÖÓ¹Ì¶¨ÎªHCLKÊ±ÖÓµÄ1/8
//SYSCLK:ÏµÍ³Ê±ÖÓ
void delay_init(u8 SYSCLK)
{
	SysTick->CTRL&=0xfffffffb;//bit2Çå¿Õ,Ñ¡ÔñÍâ²¿Ê±ÖÓ  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}								    
//ÑÓÊ±nms
//×¢ÒânmsµÄ·¶Î§
//SysTick->LOADÎª24Î»¼Ä´æÆ÷,ËùÒÔ,×î´óÑÓÊ±Îª:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLKµ¥Î»ÎªHz,nmsµ¥Î»Îªms
//¶Ô72MÌõ¼þÏÂ,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//Ê±¼ä¼ÓÔØ(SysTick->LOADÎª24bit)
	SysTick->VAL =0x00;           //Çå¿Õ¼ÆÊýÆ÷
	SysTick->CTRL=0x01 ;          //¿ªÊ¼µ¹Êý  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//µÈ´ýÊ±¼äµ½´ï   
	SysTick->CTRL=0x00;       //¹Ø±Õ¼ÆÊýÆ÷
	SysTick->VAL =0X00;       //Çå¿Õ¼ÆÊýÆ÷	  	    
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
    u32 tempPriority = 0x0A;
    // Step 1: Priority Grouping; Refer to PM0056 [Programming Manual], P135
    u32 VECT_KEY = 0xFA050000;
    u32 PRIG_GRP = 0x0005 << 8;
    SCB->AIRCR = (VECT_KEY | PRIG_GRP);
    
    // Step 2: Set Preemption priority and Subpriority
    
    tempPriority = (tempPriority << 20);
    NVIC->IP[EXTI0_IRQn] |= 0xA0;
    
    // Step 3: Set External Interrupt
    NVIC->ISER[0] |= 1 << EXTI0_IRQn;
    
//    uint32_t priorityEncoder = 0x00;
//    NVIC_SetPriorityGrouping(5);
//    NVIC_EnableIRQ(EXTI0_IRQn);
//    priorityEncoder = NVIC_EncodePriority(EXTI0_IRQn, 2, 2);
//    NVIC_SetPriority(EXTI0_IRQn, priorityEncoder);
}

void External_Configuration()
{
    u32 temp;
    // Step 1: 开启复用时钟
    RCC->APB2ENR |= 0x01;
    
    // Step 2: 外部中断配置；设置PA0 pin为外部中断端口
    temp = AFIO->EXTICR[1];
    temp &= 0xFFF0;
    AFIO->EXTICR[1] = temp;
    
    // Step 3: 将外部中断对应管脚的Mask取消掉
    EXTI->IMR |= 1 << 0;
    
    // Step 4: 配置2个触发寄存器（上升沿 and 下降沿）
    EXTI->FTSR |= 1 << 0;
}

void EXTI0_IRQHandler(void)
{
    delay_ms(10);
    
        if (keyFlag == 0)
        {
            GPIO_SetBits(GPIOA, GPIO_Pin_8);
            keyFlag = 1;
        }
        else
        {
            GPIO_ResetBits(GPIOA, GPIO_Pin_8);
            keyFlag = 0;
        }
    EXTI->PR = 1<<0;  //清除Line_0 上的标志位 
}

int main()
{
    #ifdef DEBUG
    debug();
    #endif

    int i = 0;

    //------------初始化------------
    RCC_Configuration();
    delay_init(72);
    GPIO_Configuration();
    USART_Configuration();
    
    Interrupt_Configuration();
    External_Configuration();

    while(1)
    {
//        if ( i >= 100 )
//          i = 0;

//        printf("%d\r\n", i);

//      delay_ms(200);
    }
}

