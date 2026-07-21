#ifndef LCD_H
#define LCD_H

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(unsigned char row, unsigned char col);
void LCD_PrintChar(char data);
void LCD_PrintString(const char *str);

#endif