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

#define RELAY_ON        0
#define RELAY_OFF       1
#define WINDOW_WIDTH    10
#define ZERO_SET_WINDOW_WIDTH   100

// Pressure measurement
int32_t gi32_sensorRead = 0;
int32_t gi32_window_a[WINDOW_WIDTH] = {0};
int32_t gi32_zeroSetWindow_a[ZERO_SET_WINDOW_WIDTH] = {0};

int32_t gi32_avrg = 0;
int32_t gi32_zeroSetAvrg = 0;

int32_t gi32_setZeroDat = 0; // Used for storing the value for setting zero
double gd_scale = 213.34278867748;
double gd_realWeight = 0;


// USART data transfer
u8 gu8_dataTxState = 0; // 数据传输状态，0为停止，1为开始传输

// Relay Control: 0 = Disable, 1 = Enable
u8 gu8_relayARMState = 0;
u8 gu8_relayFireState = 0;

unsigned long ReadCount(void);
int32_t getValue(void);
void fillWindow( int32_t window[], int length, int32_t val );
int32_t getWindowAvg( int32_t window[], int length );

void zeroSetFillWindow( int32_t window[], int length, int32_t val );
int32_t zeroSetGetWindowAvg( int32_t window[], int length );


int main()
{
    u8 u8_len, u8_t;
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
        gi32_sensorRead = getValue();
        
        fillWindow( &gi32_window_a[0], WINDOW_WIDTH, gi32_sensorRead );
        gi32_avrg = getWindowAvg( &gi32_window_a[0], WINDOW_WIDTH );
        
        zeroSetFillWindow( &gi32_zeroSetWindow_a[0], ZERO_SET_WINDOW_WIDTH, gi32_sensorRead);
        gi32_zeroSetAvrg = zeroSetGetWindowAvg( &gi32_zeroSetWindow_a[0], ZERO_SET_WINDOW_WIDTH );
        
        gd_realWeight = ( (double)(gi32_avrg) - (double)gi32_setZeroDat ) / gd_scale;

        
        printf("%f, %i\r\n", gd_realWeight, gi32_zeroSetAvrg);
        if ( 1 == gu8_dataTxState )
        {
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
            }
            printf("\n\n");
            USART_RX_STA=0;
            
            if ( (0 == strcmp(c_cmdCode_a, "START")) || (0 == strcmp(c_cmdCode_a, "start")) )
                gu8_dataTxState = 1;
            else if ( (0 == strcmp(c_cmdCode_a, "STOP")) || (0 == strcmp(c_cmdCode_a, "stop")) )
                gu8_dataTxState = 0;
            else if ( (0 == strcmp(c_cmdCode_a, "ZERO")) || (0 == strcmp(c_cmdCode_a, "zero")) )
                gi32_setZeroDat = gi32_zeroSetAvrg;
            else if ( (0 == strcmp(c_cmdCode_a, "ARM")) || (0 == strcmp(c_cmdCode_a, "arm")) )
                gu8_relayARMState = 1;
            else if ( (0 == strcmp(c_cmdCode_a, "FIRE")) || (0 == strcmp(c_cmdCode_a, "fire")) )
                gu8_relayFireState = 1;

        }
        
        if ( 1 == gu8_relayARMState )
        {// Pin output LOW will trigger the relay.
            pin_ARM = RELAY_ON ; // Relay ON
            if ( 1 == gu8_relayFireState )
            {
                pin_FIRE = RELAY_ON; // Relay ON
                delay_ms(1500); // After 500 miliseconds, clear both ARM & FIRE flags.
                pin_ARM = RELAY_OFF; //Relay OFF
                pin_FIRE = RELAY_OFF;
                gu8_relayARMState = 0;
                gu8_relayFireState = 0;
            }
        }

        delay_ms(150);
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
    //Count ^= 0x800000;
    pin_SCK = 0;
    return (Count);
}

int32_t getValue(void)
{
    u32 val_raw = 0;
    int32_t val4ret = 0;
    
    val_raw = ReadCount();
    if ( val_raw & 0x800000 )
    {
        val_raw = (~val_raw) + 0x01; // Two's complement it to get its abs value
        val4ret = 0;
        val4ret |= val_raw;
        val4ret = (~val4ret) + 0x01; // Convert 24bit Two's complement value to 32bit
    }
    else
    {
        val4ret = 0;
        val4ret |= val_raw;
    }
    
    return val4ret;
}

void fillWindow( int32_t window[], int length, int32_t val )
{
    static int count_fw = 0;
    window[count_fw] = val;
    if ( count_fw > length )
    {
        count_fw = 0;
    }
    else
    {
        count_fw ++;
    }
}


int32_t getWindowAvg( int32_t window[], int length )
{
    int count = 0;
    int32_t sum = 0;
    for (count = 0; count < length; count ++ )
    {
        sum += window[count];
    }
    
    return (sum / length);
}

void zeroSetFillWindow( int32_t window[], int length, int32_t val )
{
    static int count_fw = 0;
    window[count_fw] = val;
    if ( count_fw > length )
    {
        count_fw = 0;
    }
    else
    {
        count_fw ++;
    }
}

int32_t zeroSetGetWindowAvg( int32_t window[], int length )
{
    int count = 0;
    int32_t sum = 0;
    for (count = 0; count < length; count ++ )
    {
        sum += window[count];
    }
    
    return (sum / length);
}
