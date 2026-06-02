#ifndef INC_DISPLAYDRIVER_H_
#define INC_DISPLAYDRIVER_H_

#include "stm32f4xx_hal.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define FONT_CHAR_WIDTH 12
#define FONT16_CHAR_WIDTH 16

void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
void Display_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color);

#endif /* INC_DISPLAYDRIVER_H_ */
