#include "DisplayDriver.h"
#include "LCD_Driver.h"
#include <string.h>

static uint32_t getCellDisplayColor(CellState cell, uint8_t isAttackView)
{
    switch (cell)
    {
        case CELL_EMPTY:return LCD_COLOR_GRAY;
        case CELL_SHIP: return isAttackView ? LCD_COLOR_GRAY : LCD_COLOR_LIGHTBLUE;
        case CELL_HIT: return LCD_COLOR_RED;
        case CELL_MISS: return LCD_COLOR_DARKBLUE;
        case CELL_SUNK: return LCD_COLOR_DARKRED;
        default: return LCD_COLOR_GRAY;
    }
}

void Display_RenderGrid(Grid *grid, uint16_t originX, uint16_t originY, uint8_t isAttackView)
{
    for (uint8_t row = 0; row < GRID_SIZE; row++)
    {
        for (uint8_t col = 0; col < GRID_SIZE; col++)
        {
            uint16_t x = originX + col * GRID_CELL_SIZE;
            uint16_t y = originY + row * GRID_CELL_SIZE;
            uint32_t color = getCellDisplayColor(grid->cells[row][col], isAttackView);
            Display_FillRect(x, y, GRID_CELL_SIZE, GRID_CELL_SIZE, color);
            Display_DrawRect(x, y, GRID_CELL_SIZE, GRID_CELL_SIZE, LCD_COLOR_BLACK);
        }
    }
}

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
