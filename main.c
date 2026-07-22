#include "mcc_generated_files/system/system.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "functions.h"

#define ADC_THRESHOLD_3V2 1310

volatile uint8_t LoRaState = 0;
volatile uint8_t LoRaSeconds = 0;
volatile uint8_t SendMessage = 0;

volatile uint8_t RTCReadFlag = 0;

// =================================================
// IOC INTERRUPT
// =================================================

void __interrupt(irq(default), base(8)) ISR(void)
{
    if(PIR3bits.TMR0IF)
    {
        PIR3bits.TMR0IF = 0;

        TMR0H = 0xFE;
        TMR0L = 0x00;

        LoRaSeconds++;
        //LATAbits.LATA0 = !LATAbits.LATA0;
        
        RTCReadFlag = 1;
    }

    if(PIR0bits.IOCIF)
    {
        if(IOCBFbits.IOCBF3)
        {
            IOCBFbits.IOCBF3 = 0;
            A_flag = 1;
        }

        if(IOCBFbits.IOCBF4)
        {
            IOCBFbits.IOCBF4 = 0;
            B_flag = 1;
        }

        if(IOCBFbits.IOCBF5)
        {
            IOCBFbits.IOCBF5 = 0;
            C_flag = 1;
        }

        if(IOCBFbits.IOCBF6)
        {
            IOCBFbits.IOCBF6 = 0;
            D_flag = 1;
        }

        if(IOCBFbits.IOCBF7)
        {
            IOCBFbits.IOCBF7 = 0;
            E_flag = 1;
        }

        if(IOCCFbits.IOCCF5)
        {
            IOCCFbits.IOCCF5 = 0;
            rtcWake = true;
        }

        PIR0bits.IOCIF = 0;
    }
}

// =================================================
// MAIN
// =================================================

int main(void)
{
    int count = 0;
    int toggle = 0;
    int IRWasHigh = 0;
    int PIR_Reset = 0;

    uint8_t state = 0;

    char buffer[64];

    uint16_t adc = 0;

    uint8_t sleepHour = 0;
    uint8_t sleepMinute = 1;

    uint8_t wakeHour = 0;
    uint8_t wakeMinute = 37;

    uint8_t prevsec = 255;
    int loraStart;
    int currentTime;
    bool loraminSet = 0;
    
    bool waiting5Sec = false;
    uint8_t startSec = 0;
    
    

    SYSTEM_Initialize();
    ADC_Init();

    VREGCONbits.VREGPM = 1;

    ANSELA = 0;
    ANSELB = 0;
    ANSELD = 0;

    TRISDbits.TRISD0 = 1;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;

    ANSELDbits.ANSELD0 = 0;
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
    
    RTC_SetTime(0, 0, 0);

    while(1)
    {
       

        RTC_UpdateTime();
        /*
        if(RTCReadFlag)
        {
            RTCReadFlag = 0;
            RTC_UpdateTime();
        }
        */
        if(!loraminSet)
        {
            loraStart = hour * 60 + min;
            loraminSet = 1;
        }

        currentTime = hour * 60 + min;

        if(currentTime < loraStart)
        {
            currentTime += 24 * 60;
        }

        if(currentTime - loraStart >= 1)
        {
            
            UART_SendString("hello");
            loraStart = hour * 60 + min;
        }
        
        
        
        
        
        if(prevsec != sec){
            prevsec = sec;
            printf("Time: %02u:%02u:%02u\r\n", hour, min, sec);
        }
      
        if(
            (hour > sleepHour || (hour == sleepHour && min >= sleepMinute))
            &&
            (hour < wakeHour || (hour == wakeHour && min < wakeMinute))
        )
        {
            LATDbits.LATD2 = 0;
            printf("Sleeping");
            RTC_SleepUntil(wakeHour, wakeMinute, 0);
            LATDbits.LATD2 = 1;
        }
        
        
       /*
        if(LoRaState == 0)
        {
            if(LoRaSeconds >= 5)
            {
                SendMessage = 1;

                LoRaSeconds = 0;

                LoRaState = 1;
            }
        }

        if(LoRaState == 1)
        {
            if(LoRaSeconds >= 20)
            {
                LATDbits.LATD2 = 0;

                LoRaSeconds = 0;

                LoRaState = 2;
            }
        }

        if(LoRaState == 2)
        {
            if(LoRaSeconds >= 60)
            {
                LATDbits.LATD2 = 1;

                LoRaSeconds = 0;

                LoRaState = 0;
            }
        }

        if(SendMessage)
        {
            SendMessage = 0;

            sprintf(
                buffer,
                "Gate_01,%d,%d,%d,%d,%d,%d\r\n",
                count,
                A,
                B,
                C,
                D,
                E
            );

            UART_SendString(buffer);
        }

        // =================================================
        // RECEIVE "LED" FROM LORA32
        // RESET COUNTS
        // =================================================

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
                        // Reset counters

                        count = 0;

                        A = 0;
                        B = 0;
                        C = 0;
                        D = 0;
                        E = 0;

                        // Prevent false PIR count

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

        // =================================================
        // PIR COUNTING
        // =================================================

        if(PIR_Reset == 1)
        {
            // Ignore PIR until sensor clears

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

        // =================================================
        // BUTTON PROCESSING
        // =================================================

        buttonTask();
        */
    }
}