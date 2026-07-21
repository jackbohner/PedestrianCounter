#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/system/system.h"

extern volatile bool rtcWake;

extern uint8_t hour;
extern uint8_t min;
extern uint8_t sec;

extern volatile int A_flag;
extern volatile int B_flag;
extern volatile int C_flag;
extern volatile int D_flag;
extern volatile int E_flag;

extern int A;
extern int B;
extern int C;
extern int D;
extern int E;

void buttonInit(void);
void buttonTask(void);

void putch(char c);

uint8_t bcd_to_dec(uint8_t val);
uint8_t dec_to_bcd(uint8_t val);

bool I2C1_WaitDone(void);

bool DS3231_ReadTime(void);
void DS3231_PrintTime(void);
bool DS3231_WriteRegister(uint8_t reg, uint8_t value);
uint8_t DS3231_ReadRegister(uint8_t reg);
bool DS3231_SetAlarm1(uint8_t alarmHour, uint8_t alarmMin, uint8_t alarmSec);
bool DS3231_ClearAlarmFlag(void);
void DS3231_PrintAlarmDebug(void);

void Init_RTC_Interrupt_RC5(void);
void Init_RTC_Pin_RC5(void);

bool RTC_UpdateTime(void);
bool RTC_TimeReached(uint8_t targetHour, uint8_t targetMin);
void RTC_SleepUntil(uint8_t wakeHour, uint8_t wakeMin, uint8_t wakeSec);
bool RTC_SetTime(uint8_t setHour, uint8_t setMin, uint8_t setSec);

void UART_SendString(const char *str);

void ADC_Init(void);
uint16_t ADC_Read(void);

void Timer0_Init(void);


#endif