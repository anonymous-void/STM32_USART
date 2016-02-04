#include "gpio.h"
#include "stm32f10x_rcc.h"

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
    
    // HX711： DT - GPIO_A_14为MCU输入端口, SCK - GPIO_A_14端口为MCU的输出端口
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
    
    // Relay Pin Config 继电器端口：PA6 = ARM; PA7 = Fire.
    /*
        Relay is Low-Level priority, that is when GPIO = LOW, relay is enabled.
        So keep every relay pin is HIGH at the beginning of Initialiation.
    */
    // Launch ARM: Arm the firing system for safty reason.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_6);
    // Launch Fire: The realy fire command.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_7);
    
    // 按键PA0，即WK_UP键
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // 下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
