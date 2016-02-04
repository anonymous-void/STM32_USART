#include "sys.h"
#include "stm32f10x_rcc.h"

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
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE); // GPIO-A and GPIO-B clock Enabled
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // USART-1 Enabled Tx = PA9, Rx = PA10
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // USART-2 Enabled Tx = PA2, Rx = PA3

}

//////////////////////////////////////////////////////////////////////////////////
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
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
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
//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //时间加载	  		 
	SysTick->VAL=0x00;        //清空计数器
	SysTick->CTRL=0x01 ;      //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}