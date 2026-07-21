#include <xc.h>
#include "lcd.h"

#define _XTAL_FREQ 16000000UL

#define LCD_RS LATAbits.LATA0
#define LCD_E  LATAbits.LATA1

#define LCD_D4 LATAbits.LATA2
#define LCD_D5 LATAbits.LATA3
#define LCD_D6 LATAbits.LATA4
#define LCD_D7 LATAbits.LATA5

static void LCD_PulseEnable(void)
{
    LCD_E = 1;
    __delay_us(5);
    LCD_E = 0;
    __delay_us(50);
}

static void LCD_SendNibble(unsigned char nibble)
{
    LCD_D4 = (nibble >> 0) & 1;
    LCD_D5 = (nibble >> 1) & 1;
    LCD_D6 = (nibble >> 2) & 1;
    LCD_D7 = (nibble >> 3) & 1;

    LCD_PulseEnable();
}

static void LCD_Command(unsigned char cmd)
{
    LCD_RS = 0;

    LCD_SendNibble(cmd >> 4);
    LCD_SendNibble(cmd & 0x0F);

    __delay_ms(2);
}

void LCD_PrintChar(char data)
{
    LCD_RS = 1;

    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);

    __delay_us(50);
}

void LCD_PrintString(const char *str)
{
    while(*str)
    {
        LCD_PrintChar(*str++);
    }
}

void LCD_Clear(void)
{
    LCD_Command(0x01);
    __delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char col)
{
    unsigned char address;

    if(row == 1)
        address = 0x80 + (col - 1);
    else
        address = 0xC0 + (col - 1);

    LCD_Command(address);
}

void LCD_Init(void)
{
    ANSELA = 0x00;     // all PORTA digital
    WPUA = 0x00;       // no weak pullups

    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA2 = 0;
    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA5 = 0;

    LATA = 0x00;

    __delay_ms(50);

    LCD_RS = 0;
    LCD_E = 0;

    LCD_SendNibble(0x03);
    __delay_ms(5);
    LCD_SendNibble(0x03);
    __delay_ms(5);
    LCD_SendNibble(0x03);
    __delay_ms(5);
    LCD_SendNibble(0x02);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Clear();
}