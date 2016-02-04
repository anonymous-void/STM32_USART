#include "stm32f10x_rcc.h"
#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "usart.h"
#include "gpio.h"
#include "interrupt.h"

#define pin_ledGreen    PDout(2)// PD2
#define pin_DT          PAin(14)
#define pin_SCK         PAout(12)
#define pin_ARM         PAout(6)
#define pin_FIRE        PAout(7)

// Pressure measurement
u32 gu32_sensorRead = 0;
u32 gu32_window_a[10] = {0}, gu32_avrg = 0;
u32 gu32_setZeroDat = 0; // Used for storing the value for setting zero
u32 gu32_scale = 1;
u32 gu32_realWeight = 0;
double gd_window_a[10] = {0}, gd_avrg = 0;
double gd_setZeroDat = 0; // Used for storing the value for setting zero
double gd_realWeight = 0;

// USART data transfer
u8 gu8_dataTxState = 0; // 数据传输状态，0为停止，1为开始传输

// Relay Control: 0 = Disable, 1 = Enable
u8 gu8_relayARMState = 0;
u8 gu8_relayFireState = 0;

unsigned long ReadCount(void);

int main()
{
    u8 u8_len, u8_t, u8_wCnt = 0;
    char c_cmdCode_a[10] = {0};
    //------------Initialization------------
    RCC_Configuration();
    delay_init(72);
    GPIO_Configuration();
    USART1_Configuration();
    USART2_Configuration();

    Interrupt_Configuration();
    External_Configuration();

    while(1)
    {
        gu32_sensorRead = ReadCount();
        gu32_window_a[u8_wCnt] = gu32_sensorRead;
        u8_wCnt ++;
        if (u8_wCnt > 10)
        {
            u8_wCnt = 0;
        }

        gu32_avrg = (gu32_window_a[0] + gu32_window_a[1] + gu32_window_a[2] + \
                     gu32_window_a[3] + gu32_window_a[4] + gu32_window_a[5] + \
                     gu32_window_a[6] + gu32_window_a[7] + gu32_window_a[8] + \
                     gu32_window_a[9]) / 10;

        gu32_realWeight = (gu32_avrg - gu32_setZeroDat) / gu32_scale * 20;
        if ( 1 == gu8_dataTxState )
        {
            printf("w = %u, av = %u, zs = %u\r\n", gu32_sensorRead, gu32_avrg, gu32_realWeight);
            pin_ledGreen = !pin_ledGreen;
        }
        // Income Command Analyze        
        if(USART_RX_STA&0x80)
        {
            u8_len = USART_RX_STA&0x3f;
            for(u8_t = 0; u8_t < 10; u8_t ++) // Clear the cmdCode string before writting.
            {
                c_cmdCode_a[u8_t] = 0;
            }
            for(u8_t = 0; u8_t < u8_len; u8_t++)
            {
                c_cmdCode_a[u8_t] = USART_RX_BUF[u8_t]; // 将Buffer中的每个char放到cmdCode中
//                USART1->DR=USART_RX_BUF[u8_t];
//                while((USART1->SR&0X40)==0);
            }
            printf("\n\n");
            USART_RX_STA=0;
            
            if ( (0 == strcmp(c_cmdCode_a, "START")) || (0 == strcmp(c_cmdCode_a, "start")) )
                gu8_dataTxState = 1;
            else if ( (0 == strcmp(c_cmdCode_a, "STOP")) || (0 == strcmp(c_cmdCode_a, "stop")) )
                gu8_dataTxState = 0;
            else if ( (0 == strcmp(c_cmdCode_a, "ZERO")) || (0 == strcmp(c_cmdCode_a, "zero")) )
                gd_setZeroDat = gd_avrg;
            else if ( (0 == strcmp(c_cmdCode_a, "ARM")) || (0 == strcmp(c_cmdCode_a, "arm")) )
                gu8_relayARMState = 1;
            else if ( (0 == strcmp(c_cmdCode_a, "FIRE")) || (0 == strcmp(c_cmdCode_a, "fire")) )
                gu8_relayFireState = 1;

        }
        
        if ( 1 == gu8_relayARMState )
        {// Pin output LOW will trigger the relay.
            pin_ARM = 0; // Relay ON
            if ( 1 == gu8_relayFireState )
            {
                pin_FIRE = 0; // Relay ON
                delay_ms(500); // After 500 miliseconds, clear both ARM & FIRE flags.
                pin_ARM = 1; //Relay OFF
                pin_FIRE = 1;
                gu8_relayARMState = 0;
                gu8_relayFireState = 0;
            }
        }

        delay_ms(200);
    }
}

unsigned long ReadCount(void)
{
    unsigned long Count = 0;
    unsigned char i;
    pin_SCK = 0;
    Count = 0;
    while (pin_DT);
    for (i = 0; i < 24; i++)
    {
        pin_SCK = 1;
        Count = Count << 1;
        pin_SCK = 0;
        if (pin_DT)
            Count++;
    }
    pin_SCK = 1;
    Count ^= 0x800000;
    pin_SCK = 0;
    return (Count);
}
