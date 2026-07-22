#include "functions.h"

#define DS3231_ADDR 0x68

#define ADC_THRESHOLD_3V2   1310    // 1.024/3.2 * 4095

volatile bool rtcWake = false;

volatile int A_flag = 0;
volatile int B_flag = 0;
volatile int C_flag = 0;
volatile int D_flag = 0;
volatile int E_flag = 0;

int A = 0;
int B = 0;
int C = 0;
int D = 0;
int E = 0;

uint8_t hour;
uint8_t min;
uint8_t sec;

void buttonInit(void)
{
    TRISBbits.TRISB3 = 1;
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB6 = 1;
    TRISBbits.TRISB7 = 1;

    ANSELBbits.ANSELB3 = 0;
    ANSELBbits.ANSELB4 = 0;
    ANSELBbits.ANSELB5 = 0;
    ANSELBbits.ANSELB6 = 0;
    ANSELBbits.ANSELB7 = 0;

    WPUBbits.WPUB3 = 1;
    WPUBbits.WPUB4 = 1;
    WPUBbits.WPUB5 = 1;
    WPUBbits.WPUB6 = 1;
    WPUBbits.WPUB7 = 1;

    IOCBNbits.IOCBN3 = 1;
    IOCBNbits.IOCBN4 = 1;
    IOCBNbits.IOCBN5 = 1;
    IOCBNbits.IOCBN6 = 1;
    IOCBNbits.IOCBN7 = 1;

    IOCBPbits.IOCBP3 = 0;
    IOCBPbits.IOCBP4 = 0;
    IOCBPbits.IOCBP5 = 0;
    IOCBPbits.IOCBP6 = 0;
    IOCBPbits.IOCBP7 = 0;

    IOCBF = 0x00;
    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;

    INTCON0bits.GIE = 1;
}

void buttonTask(void)
{
    if(A_flag)
    {
        A_flag = 0;
        __delay_ms(30);

        if(PORTBbits.RB3 == 0)
        {
            A++;
            while(PORTBbits.RB3 == 0);
            __delay_ms(30);
        }
    }

    if(B_flag)
    {
        B_flag = 0;
        __delay_ms(30);

        if(PORTBbits.RB4 == 0)
        {
            B++;
            while(PORTBbits.RB4 == 0);
            __delay_ms(30);
        }
    }

    if(C_flag)
    {
        C_flag = 0;
        __delay_ms(30);

        if(PORTBbits.RB5 == 0)
        {
            C++;
            while(PORTBbits.RB5 == 0);
            __delay_ms(30);
        }
    }

    if(D_flag)
    {
        D_flag = 0;
        __delay_ms(30);

        if(PORTBbits.RB6 == 0)
        {
            D++;
            while(PORTBbits.RB6 == 0);
            __delay_ms(30);
        }
    }

    if(E_flag)
    {
        E_flag = 0;
        __delay_ms(30);

        if(PORTBbits.RB7 == 0)
        {
            E++;
            while(PORTBbits.RB7 == 0);
            __delay_ms(30);
        }
    }
}

void putch(char c)
{
    while(!UART2_IsTxReady());
    UART2_Write(c);
}

uint8_t bcd_to_dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

uint8_t dec_to_bcd(uint8_t val)
{
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

bool I2C1_WaitDone(void)
{
    uint32_t timeout = 500000UL;

    while(I2C1_IsBusy() && timeout > 0)
    {
        I2C1_Tasks();
        timeout--;
    }

    if(timeout == 0)
    {
        I2C1_Deinitialize();
        __delay_ms(5);
        I2C1_Initialize();
        return false;
    }

    return true;
}

bool DS3231_ReadTime(void)
{
    uint8_t reg = 0x00;
    uint8_t data[3] = {0};

    if(!I2C1_WriteRead(DS3231_ADDR, &reg, 1, data, 3))
        return false;

    if(!I2C1_WaitDone())
        return false;

    sec  = bcd_to_dec(data[0] & 0x7F);
    min  = bcd_to_dec(data[1] & 0x7F);
    hour = bcd_to_dec(data[2] & 0x3F);

    return true;
}

void DS3231_PrintTime(void)
{
    if(DS3231_ReadTime())
        printf("Time: %02u:%02u:%02u\r\n", hour, min, sec);
    else
        printf("DS3231 read failed\r\n");
}

bool DS3231_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t data[2];

    data[0] = reg;
    data[1] = value;

    if(!I2C1_Write(DS3231_ADDR, data, 2))
        return false;

    return I2C1_WaitDone();
}

uint8_t DS3231_ReadRegister(uint8_t reg)
{
    uint8_t value = 0;

    if(!I2C1_WriteRead(DS3231_ADDR, &reg, 1, &value, 1))
        return 0xFF;

    if(!I2C1_WaitDone())
        return 0xFE;

    return value;
}

bool DS3231_SetAlarm1(uint8_t alarmHour, uint8_t alarmMin, uint8_t alarmSec)
{
    uint8_t data[5];

    if(!DS3231_WriteRegister(0x0E, 0x00))
        return false;

    __delay_ms(10);

    if(!DS3231_WriteRegister(0x0F, 0x00))
        return false;

    __delay_ms(10);

    data[0] = 0x07;
    data[1] = dec_to_bcd(alarmSec);
    data[2] = dec_to_bcd(alarmMin);
    data[3] = dec_to_bcd(alarmHour);
    data[4] = 0x80;

    if(!I2C1_Write(DS3231_ADDR, data, 5))
        return false;

    if(!I2C1_WaitDone())
        return false;

    __delay_ms(10);

    return DS3231_WriteRegister(0x0E, 0x05);
}

bool DS3231_ClearAlarmFlag(void)
{
    return DS3231_WriteRegister(0x0F, 0x00);
}

void DS3231_PrintAlarmDebug(void)
{
    printf("CTRL:%02X STAT:%02X RC5:%u\r\n",
           DS3231_ReadRegister(0x0E),
           DS3231_ReadRegister(0x0F),
           PORTCbits.RC5);
}

void Init_RTC_Interrupt_RC5(void)
{
    TRISCbits.TRISC5 = 1;
    ANSELCbits.ANSELC5 = 0;
    WPUCbits.WPUC5 = 1;

    IOCCFbits.IOCCF5 = 0;
    IOCCNbits.IOCCN5 = 1;
    IOCCPbits.IOCCP5 = 0;

    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;
}

void Init_RTC_Pin_RC5(void)
{
    TRISCbits.TRISC5 = 1;
    ANSELCbits.ANSELC5 = 0;
    WPUCbits.WPUC5 = 1;

    PIE0bits.IOCIE = 0;
    PIR0bits.IOCIF = 0;
    IOCCFbits.IOCCF5 = 0;
}

bool RTC_UpdateTime(void)
{
    if(!DS3231_ReadTime())
    {
        return false;
    }

    // Reject invalid time values
    if(hour > 23 || min > 59 || sec > 59)
    {
        return false;
    }

    return true;
}

bool RTC_TimeReached(uint8_t targetHour, uint8_t targetMin)
{
    return (hour == targetHour && min == targetMin);
}

void RTC_SleepUntil(uint8_t wakeHour, uint8_t wakeMin, uint8_t wakeSec)
{
    DS3231_ClearAlarmFlag();
    __delay_ms(50);

    if(!DS3231_SetAlarm1(wakeHour, wakeMin, wakeSec))
        return;

    while(!UART2_IsTxDone());

    volatile uint8_t dummy = PORTCbits.RC5;
    (void)dummy;

    IOCCFbits.IOCCF5 = 0;
    PIR0bits.IOCIF = 0;
    PIE0bits.IOCIE = 1;

    SLEEP();
    NOP();
}

bool RTC_SetTime(uint8_t setHour, uint8_t setMin, uint8_t setSec)
{
    uint8_t data[4];

    data[0] = 0x00;
    data[1] = dec_to_bcd(setSec);
    data[2] = dec_to_bcd(setMin);
    data[3] = dec_to_bcd(setHour);   // 24-hour mode

    if(!I2C1_Write(DS3231_ADDR, data, 4))
        return false;

    return I2C1_WaitDone();
}


void UART_SendString(const char *str)
{
    while(*str)
    {

        while(!UART1_IsTxReady())
        {
        }

        UART1_Write(*str++);

    }
}

void ADC_Init(void)
{
    // Enable 1.024V Fixed Voltage Reference
    FVRCONbits.EN = 1;
    FVRCONbits.ADFVR = 0b01;
    while(!FVRCONbits.RDY);

    // ADC clock
    ADCON0bits.CS = 1;        // ADCRC
    ADCLK = 0x3F;

    // VREF+ = VDD, VREF- = VSS
    ADREFbits.PREF = 0;
    ADREFbits.NREF = 0;

    // Select FVR Buffer 1
    ADPCH = 0x3E;

    // Right justified result
    ADCON0bits.FM = 1;

    // Turn ADC on
    ADCON0bits.ON = 1;
}

uint16_t ADC_Read(void)
{
    ADPCH = 0x3E;

    __delay_us(10);          // Acquisition time

    ADCON0bits.GO = 1;
    while(ADCON0bits.GO);

    return ((uint16_t)ADRESH << 8) | ADRESL;
}

void Timer0_Init(void)
{
    T0CON0bits.T016BIT = 1;

    T0CON1bits.T0CS = 0b010;

    T0CON1bits.T0CKPS = 0b1111;

    TMR0H = 0xFE;
    TMR0L = 0x00;

    PIE3bits.TMR0IE = 1;
    PIR3bits.TMR0IF = 0;



    T0CON0bits.T0EN = 1;
}