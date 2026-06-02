#ifndef INC_DISPLAYDRIVER_H_
#define INC_DISPLAYDRIVER_H_

#include "stm32f4xx_hal.h"
#include "GameDriver.h"

#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define FONT_CHAR_WIDTH 12
#define FONT16_CHAR_WIDTH 16

#define GRID_ORIGIN_X 22
#define GRID_ORIGIN_Y 40
#define GRID_CELL_SIZE 28
#define GRID_PIXEL_WIDTH (GRID_SIZE * GRID_CELL_SIZE)

#define BTN_W 80
#define BTN_H 40
#define BTN_1P_X 20
#define BTN_1P_Y 140
#define BTN_2P_X 140
#define BTN_2P_Y 140
#define BTN_STATS_X 80
#define BTN_STATS_Y 200
#define BTN_PLACE_X 70
#define BTN_PLACE_Y 260
#define BTN_RESET_X 80
#define BTN_RESET_Y 260

void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
void Display_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_RenderGrid(Grid *grid, uint16_t originX, uint16_t originY, uint8_t isAttackView);

#endif /* INC_DISPLAYDRIVER_H_ */
