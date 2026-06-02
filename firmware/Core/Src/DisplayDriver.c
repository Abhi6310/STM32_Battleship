#include "DisplayDriver.h"
#include "LCD_Driver.h"
#include <string.h>

void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
	for (uint16_t col = x; col < x + w; col++)
		LCD_Draw_Vertical_Line(col, y, h, color);
}

void Display_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint32_t color)
{
	for (uint16_t col = x; col < x + len; col++)
		LCD_Draw_Vertical_Line(col, y, 1, color);
}

void Display_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color)
{
	Display_DrawHLine(x, y, w, color);
	Display_DrawHLine(x, y + h - 1, w, color);
	LCD_Draw_Vertical_Line(x, y, h, color);
	LCD_Draw_Vertical_Line(x + w - 1, y, h, color);
}

void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color)
{
	LCD_SetTextColor(color);
	uint16_t curX = x;
	while (*str)
	{
		LCD_DisplayChar(curX, y, (uint8_t)*str);
		curX += FONT_CHAR_WIDTH;
		str++;
	}
}

void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color)
{
	LCD_SetFont(&Font16x24);
	LCD_SetTextColor(color);
	uint16_t curX = x;
	while (*str)
	{
		LCD_DisplayChar(curX, y, (uint8_t)*str);
		curX += FONT16_CHAR_WIDTH;
		str++;
	}
	LCD_SetFont(&Font12x12);
}
