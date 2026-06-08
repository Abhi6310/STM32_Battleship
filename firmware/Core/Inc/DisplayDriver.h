#ifndef INC_DISPLAYDRIVER_H_
#define INC_DISPLAYDRIVER_H_

#include "stm32f4xx_hal.h"
#include "fonts.h"
#include "GameDriver.h"

#define Font Font12x12
#define FontLarge Font16x24

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

#define UI_BG 0x1E1E24
#define UI_INACTIVE 0x43434D
#define UI_GRID_LINE 0x2A2A30
#define UI_ACCENT 0xE5A93A
#define UI_HIT 0xD64045
#define UI_SUNK 0x5C1E22
#define UI_MISS 0x467599
#define UI_SHIP 0x6E6E78
#define UI_TEXT 0xE8E8EA
#define UI_TEXT_DIM 0x8F8F95

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
#define BTN_PLACE_W 80
#define BTN_PLACE_H 28
#define BTN_PLACE_X 80
#define BTN_PLACE_Y 270
#define BTN_RESET_X 70
#define BTN_RESET_Y 260

#define PLACEMENT_HEADER_X 20
#define PLACEMENT_HEADER_Y 12
#define SHIP_NAME_X 20
#define SHIP_NAME_Y 244
#define HINT_TEXT_X 20
#define HINT_TEXT_Y 228

#define STATS_TITLE_X 80
#define STATS_TITLE_Y 8
#define STATS_HINT_X 24
#define STATS_HINT_Y 296
#define STATS_WIN_LABEL_X 22
#define STATS_WIN_P1_Y 36
#define STATS_WIN_P2_Y 54
#define STATS_HEATMAP_CELL 22
#define STATS_HEATMAP_ORIGIN_X 43
#define STATS_HEATMAP_ORIGIN_Y 78
#define HEATMAP_LOW_MAX 1
#define HEATMAP_MID_MAX 3
#define STATS_RESET_Y 240

#define ATTACK_HEADER_X 20
#define ATTACK_HEADER_Y 12
#define STATUS_BAR_X 20
#define STATUS_BAR_Y 244
#define RESULT_OVERLAY_X 80
#define RESULT_OVERLAY_Y 260
#define GAME_OVER_TITLE_X 40
#define GAME_OVER_TITLE_Y 80
#define GAME_OVER_HINT_X 24
#define GAME_OVER_HINT_Y 180

#define TRANSITION_TITLE_X 40
#define TRANSITION_TITLE_Y 60
#define TRANSITION_HINT_X 24
#define TRANSITION_HINT_Y 100
#define STATS_LINE_Y 45

void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
void Display_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color);
void Display_RenderGrid(Grid *grid, uint16_t originX, uint16_t originY, uint8_t isAttackView);
void Display_RenderHomeScreen(void);
void Display_RenderPlacementScreen(uint8_t player);
void Display_UpdatePlacementPreview(void);
void Display_RenderStatsScreen(void);
void Display_RenderAttackScreen(uint8_t player);
void Display_ShowHitResult(uint8_t player);
void Display_ShowMissResult(uint8_t player);
void Display_RenderGameOver(uint8_t winner);
void Display_RenderTransitionScreen(const char *message);
void Display_RenderStateForTarget(GameState target);

#endif /* INC_DISPLAYDRIVER_H_ */
