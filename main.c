#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/system/system.h"
#include "functions.h"

#define ADC_THRESHOLD_3V2 1310

volatile uint8_t LoRaState = 0;
volatile uint8_t LoRaSeconds = 0;
volatile uint8_t SendMessage = 0;

void __interrupt(irq(default), base(8)) ISR(void)
{
    if(PIR3bits.TMR0IF)
    {
        PIR3bits.TMR0IF = 0;

        TMR0H = 0xFC;
        TMR0L = 0x00;

        LoRaSeconds++;
    }

    if(PIR0bits.IOCIF)
    {
        if(IOCBFbits.IOCBF3) { IOCBFbits.IOCBF3 = 0; A_flag = 1; }
        if(IOCBFbits.IOCBF4) { IOCBFbits.IOCBF4 = 0; B_flag = 1; }
        if(IOCBFbits.IOCBF5) { IOCBFbits.IOCBF5 = 0; C_flag = 1; }
        if(IOCBFbits.IOCBF6) { IOCBFbits.IOCBF6 = 0; D_flag = 1; }
        if(IOCBFbits.IOCBF7) { IOCBFbits.IOCBF7 = 0; E_flag = 1; }

        if(IOCCFbits.IOCCF5)
        {
            IOCCFbits.IOCCF5 = 0;
            rtcWake = true;
        }

        PIR0bits.IOCIF = 0;
    }
}

int main(void)
{
    int count = 0;
    int toggle = 0;
    int IRWasHigh = 0;
    int PIR_Reset = 0;
    uint8_t state = 0;
    char buffer[64];
    uint16_t adc = 0;

    uint8_t sleepHour = 21;
    uint8_t sleepMinute = 0;

    uint8_t wakeHour = 7;
    uint8_t wakeMinute = 0;

    uint8_t prevsec = 255;

    SYSTEM_Initialize();
    ADC_Init();

    VREGCONbits.VREGPM = 1;

    ANSELA = 0;
    ANSELB = 0;
    ANSELD = 0;

    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;

    ANSELDbits.ANSELD2 = 0;
    ANSELDbits.ANSELD3 = 0;

    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 0;

    LATDbits.LATD2 = 1;
    LATDbits.LATD3 = 1;

    LoRaState = 0;
    LoRaSeconds = 0;
    SendMessage = 0;

    Timer0_Init();
    buttonInit();

    Init_RTC_Interrupt_RC5();
    DS3231_ClearAlarmFlag();
    __delay_ms(50);

    INTERRUPT_GlobalInterruptEnable();

    while(1)
    {
        RTC_UpdateTime();

        if(prevsec != sec)
        {
            //adc = ADC_Read();
            prevsec = sec;
            printf("Time: %02u:%02u:%02u\r\n", hour, min, sec);
            printf("%u\r\n", adc);
        }

        if(
            (hour > sleepHour || (hour == sleepHour && min >= sleepMinute))
            ||
            (hour < wakeHour || (hour == wakeHour && min < wakeMinute))
        )
        {
            LATDbits.LATD3 = 0;
            printf("Sleeping...\r\n");
            while(!UART2_IsTxDone());

            RTC_SleepUntil(wakeHour, wakeMinute, 0);
            LATDbits.LATD3 = 1;
        }

        /*
         * - LoRa power begins ON.
         * - Send after 5 seconds.
         * - Turn LoRa OFF 20 seconds after sending.
         * - Keep it OFF for 20 seconds.
         * - Turn it ON and repeat.
         */
        if(LoRaState == 0)
        {
            if(LoRaSeconds >= 5)
            {
                SendMessage = 1;
                LoRaSeconds = 0;
                LoRaState = 1;
            }
        }
        else if(LoRaState == 1)
        {
            if(LoRaSeconds >= 20)
            {
                LATDbits.LATD2 = 0;
                LoRaSeconds = 0;
                LoRaState = 2;
            }
        }
        else if(LoRaState == 2)
        {
            if(LoRaSeconds >= 20)
            {
                LATDbits.LATD2 = 1;
                LoRaSeconds = 0;
                LoRaState = 0;
            }
        }

        if(SendMessage)
        {
            SendMessage = 0;

            sprintf(buffer,
                    "Gate_01,%d,%d,%d,%d,%d,%d\r\n",
                    count,
                    A,
                    B,
                    C,
                    D,
                    E);

            UART_SendString(buffer);
        }

        if(UART1_IsRxReady())
        {
            char c = UART1_Read();

            switch(state)
            {
                case 0:
                    if(c == 'L')
                        state = 1;
                    break;

                case 1:
                    if(c == 'E')
                        state = 2;
                    else if(c == 'L')
                        state = 1;
                    else
                        state = 0;
                    break;

                case 2:
                    if(c == 'D')
                    {
                        count = 0;
                        A = 0;
                        B = 0;
                        C = 0;
                        D = 0;
                        E = 0;

                        IRWasHigh = 0;
                        PIR_Reset = 1;
                    }

                    state = 0;
                    break;

                default:
                    state = 0;
                    break;
            }
        }

        if(PIR_Reset == 1)
        {
            LATAbits.LATA0 = 0;

            if(PORTDbits.RD0 == 0)
            {
                PIR_Reset = 0;
                IRWasHigh = 0;
            }
        }
        else
        {
            if(PORTDbits.RD0 == 1)
            {
                LATAbits.LATA0 = 1;
                IRWasHigh = 1;
            }
            else
            {
                LATAbits.LATA0 = 0;

                if(IRWasHigh == 1)
                {
                    count++;
                    IRWasHigh = 0;
                }
            }
        }
        
        buttonTask();
        int targetsec = 0;
        if (min == 0 && toggle == 0){
            toggle = 1;
            adc = ADC_Read();
                sprintf(buffer,"Gate_01,%d,%d,%d,%d,%d,%d,%02u\r\n",count,A,B,C,D,E,adc);
                if(adc >= ADC_THRESHOLD_3V2)
                {
                    LATDbits.LATD2 = 1;
                    if (sec > 52){
                        targetsec = sec - 52;
                    }
                    else{
                        targetsec = sec + 8;
                    }
                    while(sec != targetsec){
                        
                    }
                    UART_SendString(buffer);
                    RTC_UpdateTime();
                    if (sec > 52){
                        targetsec = sec - 52;
                    }
                    else{
                        targetsec = sec + 8;
                    }
                    while(sec != targetsec){
                        
                    }
                    LATDbits.LATD2 = 0;
                }
        }
        if (min != 0 && toggle == 1){
            toggle = 0;
        }
    }
}