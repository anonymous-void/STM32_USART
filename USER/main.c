#include "stm32f10x_rcc.h"
//#include "stm32f10x_gpio.h"

USART_InitTypeDef USART_InitStructure;


int main()
{
    
    return 0;
}

static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数
//初始化延迟函数
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init(u8 SYSCLK)
{
	SysTick->CTRL&=0xfffffffb;//bit2清空,选择外部时钟  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}								    

void delay_ms(u16 nms)
{	 		  	  
	u32 temp;
	SysTick->LOAD=(u32)nms*fac_ms;//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
    SysTick->VAL =0X00;       //清空计数器	  	    
} 

void RCC_Configuration(void)
{
    RCC_DeInit();
    RCC_HSEConfig(RCC_HSE_ON);
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); // Wait for External Clock stablize.
    RCC_HCLKConfig(RCC_SYSCLK_Div1); // Advance High Speed Bus AHB prescaler = /1
    RCC_PCLK1Config(RCC_HCLK_Div2); // Low-speed peripheral APB1 prescaler = /2
    RCC_PCLK2Config(RCC_HCLK_Div1); // High-speed peripheral APB2 prescaler = /1
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // 
    RCC_PLLCmd( ENABLE );
    while ( RCC_GetSYSCLKSource() != 0x08 );
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE); // GPIO-A and GPIO-B clock Enabled
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // USART-1 Enabled
    
}

void USART_Configuration(void)
{
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_StopBits_1;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}
