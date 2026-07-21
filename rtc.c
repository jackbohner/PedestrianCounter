#include <xc.h>
#include <stdint.h>
#include "rtc.h"

#define _XTAL_FREQ 4000000UL

#define DS3231_ADDR 0x68

#define SCL_LAT LATBbits.LATB1
#define SDA_LAT LATBbits.LATB2

#define SCL_TRIS TRISBbits.TRISB1
#define SDA_TRIS TRISBbits.TRISB2

#define SDA_PORT PORTBbits.RB2

static void I2C_Delay(void)
{
    __delay_us(10);
}

static void SDA_Low(void)
{
    SDA_LAT = 0;
    SDA_TRIS = 0;
}

static void SDA_Release(void)
{
    SDA_TRIS = 1;
}

static void SCL_Low(void)
{
    SCL_LAT = 0;
    SCL_TRIS = 0;
}

static void SCL_Release(void)
{
    SCL_TRIS = 1;
}

static void I2C_Start(void)
{
    SDA_Release();
    SCL_Release();
    I2C_Delay();

    SDA_Low();
    I2C_Delay();

    SCL_Low();
}

static void I2C_Stop(void)
{
    SDA_Low();
    I2C_Delay();

    SCL_Release();
    I2C_Delay();

    SDA_Release();
    I2C_Delay();
}

static void I2C_WriteByte(uint8_t data)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        if(data & 0x80)
            SDA_Release();
        else
            SDA_Low();

        I2C_Delay();
        SCL_Release();
        I2C_Delay();
        SCL_Low();

        data <<= 1;
    }

    SDA_Release();
    I2C_Delay();

    SCL_Release();
    I2C_Delay();

    SCL_Low();
}

static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t data = 0;

    SDA_Release();

    for(uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;

        SCL_Release();
        I2C_Delay();

        if(SDA_PORT)
            data |= 1;

        SCL_Low();
        I2C_Delay();
    }

    if(ack)
        SDA_Low();
    else
        SDA_Release();

    I2C_Delay();
    SCL_Release();
    I2C_Delay();
    SCL_Low();

    SDA_Release();

    return data;
}

static uint8_t decToBcd(uint8_t val)
{
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

static uint8_t bcdToDec(uint8_t val)
{
    return (uint8_t)(((val >> 4) * 10) + (val & 0x0F));
}

void RTC_Init(void)
{
    ANSELBbits.ANSELB1 = 0;
    ANSELBbits.ANSELB2 = 0;

    LATBbits.LATB1 = 0;
    LATBbits.LATB2 = 0;

    SCL_Release();
    SDA_Release();
}

void RTC_SetTime(
    uint8_t hour,
    uint8_t min,
    uint8_t sec,
    uint8_t day,
    uint8_t date,
    uint8_t month,
    uint8_t year
)
{
    I2C_Start();

    I2C_WriteByte((DS3231_ADDR << 1) | 0);
    I2C_WriteByte(0x00);

    I2C_WriteByte(decToBcd(sec));
    I2C_WriteByte(decToBcd(min));
    I2C_WriteByte(decToBcd(hour));
    I2C_WriteByte(decToBcd(day));
    I2C_WriteByte(decToBcd(date));
    I2C_WriteByte(decToBcd(month));
    I2C_WriteByte(decToBcd(year));

    I2C_Stop();
}

void RTC_ReadTime(uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    uint8_t rawSec;
    uint8_t rawMin;
    uint8_t rawHour;

    I2C_Start();

    I2C_WriteByte((DS3231_ADDR << 1) | 0);
    I2C_WriteByte(0x00);

    I2C_Start();

    I2C_WriteByte((DS3231_ADDR << 1) | 1);

    rawSec  = I2C_ReadByte(1);
    rawMin  = I2C_ReadByte(1);
    rawHour = I2C_ReadByte(0);

    I2C_Stop();

    *sec  = bcdToDec(rawSec & 0x7F);
    *min  = bcdToDec(rawMin);
    *hour = bcdToDec(rawHour & 0x3F);
}