#include "stm32f10x_rcc.h"
#include <stdio.h>
//#include "stm32f10x_gpio.h"

///////////////////////////////////////////////////////////////
//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //输出 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //输入 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //输出 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //输入

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //输出 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //输入

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //输出 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //输入

#define LED1    PDout(2)// PD2
#define DT      PAin(14)
#define SCK     PAout(12)



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
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE); // GPIO-A and GPIO-B clock Enabled
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // USART-1 Enabled Tx = PA9, Rx = PA10
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  // USART-2 Enabled Tx = PA2, Rx = PA3

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
    USART_InitStructure.USART_Mode = USART_Mode_Tx;
    
    USART_Init(USART2, &USART_InitStructure);
    
    USART_Cmd(USART2, ENABLE);
}

void GPIO_Configuration(void)
{
    //CLK:PB5  CLR:PE11  Data:PE10
    GPIO_InitTypeDef        GPIO_InitStructure;                //声明一个结构体变量

    // USART_1 Pin Config
    //配置USART1_Tx为复合推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //配置 USART1_Rx 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // USART_2 Pin Config
    //配置USART2_Tx为复合推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //配置 USART2_Rx 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // HX711： DT - GPIO_A_14为MCU输入端口, SCK - GPIO_A_15端口为MCU的输出端口
    // DT
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;  // DT
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 默认下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // SCK
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  // SCK
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // LED Pin Config
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    
    // 按键PA0，即WK_UP键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
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

while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
        USART2->DR = (u8) ch;          

return ch; 
} 
#endif 
//加入以上代码,支持printf函数,而不需要选择use MicroLIB 

void Interrupt_Configuration()
{ 
    uint32_t priorityEncoder = 0x00;
    NVIC_SetPriorityGrouping(5);
    NVIC_EnableIRQ(EXTI0_IRQn);
    priorityEncoder = NVIC_EncodePriority(5, 2, 0);
    NVIC_SetPriority(EXTI0_IRQn, priorityEncoder);
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

unsigned long ReadCount(void)
{
    unsigned long Count;
    unsigned char i;
    SCK = 0;
    Count = 0;
    while (DT);
    for (i = 0; i < 24; i++)
    {
        SCK = 1;
        Count = Count << 1;
        SCK = 0;
        if (DT)
            Count++;
    }
    SCK = 1;
    Count ^= 0x800000;
    SCK = 0;
    return (Count);
}

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

 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if(TIM3->SR&0X0001)//
	{
        //See = !See;
		LED1=(!LED1);			    				   				     	    	
	}				   
	TIM3->SR&=~(1<<0);//	    
}

void Timerx_Init(u16 arr,u16 psc)
{
    uint32_t priorityEncoder = 0x00;
	RCC->APB1ENR|=1<<1;// 
 	TIM3->ARR=arr;  //
	TIM3->PSC=psc;  //

	TIM3->DIER|=1<<0;   //ÔÊÐí¸üÐÂÖÐ¶Ï				
	TIM3->CR1|=0x01;    
  	
    NVIC_SetPriorityGrouping(5);
    NVIC_EnableIRQ(TIM3_IRQn);
    priorityEncoder = NVIC_EncodePriority(5, 0, 0);
    NVIC_SetPriority(TIM3_IRQn, priorityEncoder);								 
}

int main()
{
    #ifdef DEBUG
    debug();
    #endif

    u32 weight = 0;

    //------------初始化------------
    RCC_Configuration();
    delay_init(72);
    GPIO_Configuration();
//    USART1_Configuration();
//    USART2_Configuration();
//    
//    Interrupt_Configuration();
//    External_Configuration();
    Timerx_Init(35999, 1);

    while(1)
    {
//        weight = ReadCount();
//        printf("w = %i\r\n", weight);
//        delay_ms(100);
//        USART1_send('s');
//        USART2_send('s');
//        printf("laflal\n");
//        LED1 = ~LED1;
//        delay_ms(200);
        
    }
}

