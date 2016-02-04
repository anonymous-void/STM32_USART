#include <stm32f10x_rcc.h>
#include "interrupt.h"
#include "usart.h"
#include "sys.h"


void Interrupt_Configuration()
{ 
    uint32_t priorityEncoder = 0x00;
    
    NVIC_SetPriorityGrouping(5);
    
    NVIC_EnableIRQ(EXTI0_IRQn);
    priorityEncoder = NVIC_EncodePriority(5, 2, 0);
    NVIC_SetPriority(EXTI0_IRQn, priorityEncoder);
    
    NVIC_EnableIRQ(USART1_IRQn);
    priorityEncoder = NVIC_EncodePriority(5, 2, 1);
    NVIC_SetPriority(USART1_IRQn, priorityEncoder);
    
    NVIC_EnableIRQ(USART2_IRQn);
    priorityEncoder = NVIC_EncodePriority(5, 2, 2);
    NVIC_SetPriority(USART2_IRQn, priorityEncoder);
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
    if (gu8_dataTxState == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        gu8_dataTxState = 1;
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        gu8_dataTxState = 0;
    }
    
    EXTI->PR = 1<<0;  //清除Line_0 上的标志位 
}
