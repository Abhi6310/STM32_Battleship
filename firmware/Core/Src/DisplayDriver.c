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

void Display_RenderHomeScreen(void)
{
	LCD_Clear(LCD_COLOR_BLACK);
	Display_DrawStringLarge(40, 20, "BATTLESHIP", LCD_COLOR_CYAN);

	Display_FillRect(HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_BLUE);
	Display_DrawRect(HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(HOME_BTN_X + 28, BTN_SINGLE_Y + 14, "Singleplayer", LCD_COLOR_WHITE);

	Display_FillRect(HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_DARKGREEN);
	Display_DrawRect(HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(HOME_BTN_X + 34, BTN_MULTI_Y + 14, "Multiplayer", LCD_COLOR_WHITE);

	Display_FillRect(HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_ORANGE);
	Display_DrawRect(HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(HOME_BTN_X + 70, BTN_STATS_Y + 14, "Stats", LCD_COLOR_WHITE);
}

void Display_RenderPlacementPlaceholder(uint8_t isMultiplayer)
{
	LCD_Clear(LCD_COLOR_DARKBLUE);
	Display_DrawStringLarge(48, 130, "PLACEMENT", LCD_COLOR_WHITE);
	Display_DrawString(isMultiplayer ? 60 : 72, 170,
		isMultiplayer ? "Multiplayer" : "Singleplayer", LCD_COLOR_WHITE);
	Display_DrawString(24, 210, "Hold button 3s for home", LCD_COLOR_WHITE);
}

void Display_RenderStatsPlaceholder(void)
{
	LCD_Clear(LCD_COLOR_ORANGE);
	Display_DrawStringLarge(80, 130, "STATS", LCD_COLOR_BLACK);
	Display_DrawString(24, 210, "Hold button 3s for home", LCD_COLOR_BLACK);
}
