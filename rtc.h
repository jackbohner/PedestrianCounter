#ifndef RTC_H
#define RTC_H

#include <stdint.h>

void RTC_Init(void);

void RTC_SetTime(
    uint8_t hour,
    uint8_t min,
    uint8_t sec,
    uint8_t day,
    uint8_t date,
    uint8_t month,
    uint8_t year
);

void RTC_ReadTime(uint8_t *hour, uint8_t *min, uint8_t *sec);

#endif