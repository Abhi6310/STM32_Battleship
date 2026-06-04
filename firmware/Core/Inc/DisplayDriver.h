#ifndef INC_DISPLAYDRIVER_H_
#define INC_DISPLAYDRIVER_H_

#include "stm32f4xx_hal.h"
#include "fonts.h"
#include "GameDriver.h"

#define Font Font12x12
#define FontLarge Font16x24

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

#define GRID_ORIGIN_X 22
#define GRID_ORIGIN_Y 40
#define GRID_CELL_SIZE 28
#define GRID_PIXEL_WIDTH (GRID_SIZE * GRID_CELL_SIZE)

#define HOME_TITLE_X 40
#define HOME_TITLE_Y 20
#define HOME_BTN_X 20
#define HOME_BTN_W 200
#define HOME_BTN_H 40
#define BTN_SINGLE_Y 80
#define BTN_MULTI_Y 140
#define BTN_STATS_Y 200
#define BTN_TEXT_Y_OFFSET 14
#define BTN_W 100
#define BTN_H 40
#define BTN_PLACE_X 70
#define BTN_PLACE_Y 260
#define BTN_RESET_X 80
#define BTN_RESET_Y 260

#define PLACEMENT_HEADER_X 20
#define PLACEMENT_HEADER_Y 12
#define SHIP_NAME_X 20
#define SHIP_NAME_Y 244
#define HINT_TEXT_X 20
#define HINT_TEXT_Y 228

#define STATS_TITLE_X 80
#define STATS_TITLE_Y 130
#define STATS_HINT_X 24
#define STATS_HINT_Y 210

void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
void Display_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_RenderGrid(Grid *grid, uint16_t originX, uint16_t originY, uint8_t isAttackView);
void Display_RenderHomeScreen(void);
void Display_RenderPlacementScreen(uint8_t player);
void Display_UpdatePlacementPreview(void);
void Display_RenderStatsPlaceholder(void);

#endif /* INC_DISPLAYDRIVER_H_ */
