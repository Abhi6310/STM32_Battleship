#include "DisplayDriver.h"
#include "LCD_Driver.h"
#include <string.h>

#define DEC_DIGITS_U32 10
#define BTN_BORDER_INSET 1
#define MARKER_HIT_RADIUS 9
#define MARKER_SUNK_OUTER 10
#define MARKER_SUNK_INNER 5
#define MARKER_MISS_HALF 3
#define SHIP_CELL_INSET 3
#define PREVIEW_CELL_INSET 2
#define RESULT_PANEL_W 120
#define RESULT_PANEL_H 32
#define TITLE_UNDERLINE_GAP 28
#define GAME_OVER_MARKER_DROP 60
#define GAME_OVER_MARKER_R 8

static uint16_t centerTextX(uint16_t boxX, uint16_t boxW, const char *str)
{
	uint16_t textPx = (uint16_t)strlen(str) * Font.Width;
	return boxX + (boxW - textPx) / 2;
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
	while (*str) {
		LCD_DisplayChar(curX, y, (uint8_t)*str);
		curX += Font.Width;
		str++;
	}
}

void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color)
{
	LCD_SetFont(&FontLarge);
	LCD_SetTextColor(color);
	uint16_t curX = x;
	while (*str) {
		LCD_DisplayChar(curX, y, (uint8_t)*str);
		curX += FontLarge.Width;
		str++;
	}
	LCD_SetFont(&Font);
}

static void drawAccentButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *label, uint32_t accent)
{
	Display_FillRect(x, y, w, h, UI_INACTIVE);
	Display_DrawRect(x, y, w, h, accent);
	Display_DrawRect(x + BTN_BORDER_INSET, y + BTN_BORDER_INSET, w - 2 * BTN_BORDER_INSET, h - 2 * BTN_BORDER_INSET, accent);
	uint16_t textPx = (uint16_t)strlen(label) * Font.Width;
	uint16_t textX = x + (w - textPx) / 2;
	uint16_t textY = y + (h - Font.Height) / 2;
	Display_DrawString(textX, textY, label, accent);
}

static void drawTitleWithUnderline(const char *title, uint16_t y, uint32_t color)
{
	uint16_t titleW = (uint16_t)strlen(title) * FontLarge.Width;
	uint16_t titleX = (LCD_WIDTH - titleW) / 2;
	Display_DrawStringLarge(titleX, y, title, color);
	Display_FillRect(titleX, y + TITLE_UNDERLINE_GAP, titleW, 2, color);
}

void Display_RenderGrid(Grid *grid, uint16_t originX, uint16_t originY, uint8_t isAttackView)
{
	for (uint8_t row = 0; row < GRID_SIZE; row++) {
		for (uint8_t col = 0; col < GRID_SIZE; col++) {
			uint16_t x = originX + col * GRID_CELL_SIZE;
			uint16_t y = originY + row * GRID_CELL_SIZE;
			CellState cell = grid->cells[row][col];
			uint16_t cx = x + GRID_CELL_SIZE / 2;
			uint16_t cy = y + GRID_CELL_SIZE / 2;
			Display_FillRect(x, y, GRID_CELL_SIZE, GRID_CELL_SIZE, UI_INACTIVE);
			Display_DrawRect(x, y, GRID_CELL_SIZE, GRID_CELL_SIZE, UI_GRID_LINE);
			switch (cell) {
				case CELL_SHIP:
					if (!isAttackView) {
						Display_FillRect(x + SHIP_CELL_INSET, y + SHIP_CELL_INSET, GRID_CELL_SIZE - 2 * SHIP_CELL_INSET, GRID_CELL_SIZE - 2 * SHIP_CELL_INSET, UI_SHIP);
					}
					break;
				case CELL_HIT:
					LCD_Draw_Circle_Fill(cx, cy, MARKER_HIT_RADIUS, UI_HIT);
					break;
				case CELL_SUNK:
					LCD_Draw_Circle_Fill(cx, cy, MARKER_SUNK_OUTER, UI_SUNK);
					LCD_Draw_Circle_Fill(cx, cy, MARKER_SUNK_INNER, UI_HIT);
					break;
				case CELL_MISS:
					Display_FillRect(cx - MARKER_MISS_HALF, cy - MARKER_MISS_HALF, 2 * MARKER_MISS_HALF, 2 * MARKER_MISS_HALF, UI_MISS);
					break;
				default:
					break;
			}
		}
	}
}

void Display_RenderHomeScreen(void)
{
	LCD_Clear(UI_BG);
	drawTitleWithUnderline("BATTLESHIP", HOME_TITLE_Y, UI_ACCENT);
	drawAccentButton(HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H, "SINGLEPLAYER", UI_ACCENT);
	drawAccentButton(HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H, "MULTIPLAYER", UI_ACCENT);
	drawAccentButton(HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H, "STATS", UI_TEXT_DIM);
}

void Display_RenderPlacementScreen(uint8_t player)
{
	LCD_Clear(UI_BG);
	const char *header = (player == 2) ? "P2: PLACE SHIPS" : "P1: PLACE SHIPS";
	Display_DrawString(centerTextX(0, LCD_WIDTH, header), PLACEMENT_HEADER_Y, header, UI_TEXT);
	Grid *activeGrid = (player == 2) ? GameDriver_GetAIGrid() : GameDriver_GetPlayerGrid();
	Display_RenderGrid(activeGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, 0);
	uint8_t shipIdx = GameDriver_GetPlacingShipIndex();
	if (shipIdx < NUM_SHIPS) {
		const char *names[NUM_SHIPS] = { "DESTROYER (2)", "SUBMARINE (3)", "BATTLESHIP (4)" };
		Display_DrawString(centerTextX(0, LCD_WIDTH, names[shipIdx]), SHIP_NAME_Y, names[shipIdx], UI_ACCENT);
	}
	drawAccentButton(BTN_PLACE_X, BTN_PLACE_Y, BTN_PLACE_W, BTN_PLACE_H, "PLACE", UI_ACCENT);
}

void Display_UpdatePlacementPreview(void)
{
	uint8_t isP2 = (GameDriver_GetState() == STATE_P2_PLACEMENT);
	Grid *activeGrid = isP2 ? GameDriver_GetAIGrid() : GameDriver_GetPlayerGrid();
	Ship *pendingShip = GameDriver_GetPlacingShip();
	Display_RenderGrid(activeGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, 0);
	uint32_t previewColor = GameDriver_IsPlacementValid(activeGrid, pendingShip) ? UI_ACCENT : UI_HIT;
	for (uint8_t i = 0; i < pendingShip->length; i++) {
		uint8_t r, c;
		GameDriver_GetShipCell(pendingShip, i, &r, &c);
		if (r < GRID_SIZE && c < GRID_SIZE) {
			uint16_t px = GRID_ORIGIN_X + c * GRID_CELL_SIZE;
			uint16_t py = GRID_ORIGIN_Y + r * GRID_CELL_SIZE;
			Display_FillRect(px + PREVIEW_CELL_INSET, py + PREVIEW_CELL_INSET, GRID_CELL_SIZE - 2 * PREVIEW_CELL_INSET, GRID_CELL_SIZE - 2 * PREVIEW_CELL_INSET, previewColor);
		}
	}
}

static void u32ToStr(uint32_t num, char *buf)
{
	char tmp[DEC_DIGITS_U32];
	uint8_t n = 0;
	if (num == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}
	while (num > 0 && n < DEC_DIGITS_U32) {
		tmp[n++] = (char)('0' + (num % 10));
		num /= 10;
	}
	for (uint8_t i = 0; i < n; i++) {
		buf[i] = tmp[n - 1 - i];
	}
	buf[n] = '\0';
}

static uint8_t appendStr(char *dst, uint8_t i, uint8_t cap, const char *src)
{
	while (*src && i < cap - 1) dst[i++] = *src++;
	return i;
}

static void drawWinLineSingle(uint16_t y, uint32_t p1, uint32_t ai)
{
	char p1Buf[DEC_DIGITS_U32 + 1];
	char aiBuf[DEC_DIGITS_U32 + 1];
	char line[32];
	u32ToStr(p1, p1Buf);
	u32ToStr(ai, aiBuf);
	uint8_t i = 0;
	i = appendStr(line, i, sizeof(line), "P1: ");
	i = appendStr(line, i, sizeof(line), p1Buf);
	i = appendStr(line, i, sizeof(line), "   AI: ");
	i = appendStr(line, i, sizeof(line), aiBuf);
	line[i] = '\0';
	Display_DrawString(centerTextX(0, LCD_WIDTH, line), y, line, UI_TEXT);
}

static uint32_t heatmapColor(uint32_t hits)
{
	if (hits == 0) return UI_INACTIVE;
	if (hits <= HEATMAP_LOW_MAX) return UI_MISS;
	if (hits <= HEATMAP_MID_MAX) return UI_ACCENT;
	return UI_HIT;
}

void Display_RenderStatsScreen(void)
{
	GameStats *stats = GameDriver_GetStats();
	LCD_Clear(UI_BG);
	drawTitleWithUnderline("STATS", STATS_TITLE_Y, UI_ACCENT);
	drawWinLineSingle(STATS_LINE_Y, stats->playerWins, stats->aiWins);
	for (uint8_t row = 0; row < GRID_SIZE; row++) {
		for (uint8_t col = 0; col < GRID_SIZE; col++) {
			uint16_t px = STATS_HEATMAP_ORIGIN_X + col * STATS_HEATMAP_CELL;
			uint16_t py = STATS_HEATMAP_ORIGIN_Y + row * STATS_HEATMAP_CELL;
			Display_FillRect(px + 1, py + 1, STATS_HEATMAP_CELL - 2, STATS_HEATMAP_CELL - 2, heatmapColor(stats->heatmap[row][col]));
		}
	}
	drawAccentButton(BTN_RESET_X, STATS_RESET_Y, BTN_W, BTN_H, "RESET", UI_ACCENT);
	Display_DrawString(centerTextX(0, LCD_WIDTH, "PRESS BTN FOR HOME"), STATS_HINT_Y, "PRESS BTN FOR HOME", UI_TEXT_DIM);
}

static void renderShipStatusBar(uint8_t player)
{
	Ship *ownShips = (player == 2) ? GameDriver_GetAIShips() : GameDriver_GetPlayerShips();
	static const char *labels[NUM_SHIPS] = { "DES", "SUB", "BAT" };
	uint16_t curX = STATUS_BAR_X;
	for (uint8_t i = 0; i < NUM_SHIPS; i++) {
		uint32_t color;
		if (ownShips[i].hitCount == 0) color = UI_TEXT;
		else if (ownShips[i].hitCount >= ownShips[i].length) color = UI_SUNK;
		else color = UI_ACCENT;
		Display_DrawString(curX, STATUS_BAR_Y, labels[i], color);
		curX += (uint16_t)strlen(labels[i]) * Font.Width + Font.Width;
	}
}

void Display_RenderAttackScreen(uint8_t player)
{
	LCD_Clear(UI_BG);
	const char *header;
	Grid *targetGrid;
	uint8_t isAttackView;
	if (player == 0) {
		header = "AI TURN";
		targetGrid = GameDriver_GetPlayerGrid();
		isAttackView = 0;
	} else if (player == 2) {
		header = "P2: TAP TO ATTACK";
		targetGrid = GameDriver_GetPlayerGrid();
		isAttackView = 1;
	} else {
		header = GameDriver_IsMultiplayer() ? "P1: TAP TO ATTACK" : "TAP TO ATTACK";
		targetGrid = GameDriver_GetAIGrid();
		isAttackView = 1;
	}
	Display_DrawString(centerTextX(0, LCD_WIDTH, header), ATTACK_HEADER_Y, header, UI_TEXT);
	Display_RenderGrid(targetGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, isAttackView);
	renderShipStatusBar(player);
}

static void drawResultPanel(const char *text, uint32_t fill)
{
	const uint16_t px = (LCD_WIDTH - RESULT_PANEL_W) / 2;
	const uint16_t py = RESULT_OVERLAY_Y;
	Display_FillRect(px, py, RESULT_PANEL_W, RESULT_PANEL_H, fill);
	Display_DrawRect(px, py, RESULT_PANEL_W, RESULT_PANEL_H, UI_ACCENT);
	Display_DrawRect(px + 1, py + 1, RESULT_PANEL_W - 2, RESULT_PANEL_H - 2, UI_ACCENT);
	uint16_t textPx = (uint16_t)strlen(text) * FontLarge.Width;
	Display_DrawStringLarge(px + (RESULT_PANEL_W - textPx) / 2, py + 4, text, UI_TEXT);
}

void Display_ShowHitResult(uint8_t player)
{
	Display_RenderAttackScreen(player);
	drawResultPanel("HIT", UI_HIT);
}

void Display_ShowMissResult(uint8_t player)
{
	Display_RenderAttackScreen(player);
	drawResultPanel("MISS", UI_MISS);
}

void Display_RenderGameOver(uint8_t winner)
{
	const char *title;
	uint32_t titleColor;
	if (winner == 1) {
		title = "P1 WINS";
		titleColor = UI_ACCENT;
	} else if (winner == 2) {
		title = "P2 WINS";
		titleColor = UI_ACCENT;
	} else {
		title = "AI WINS";
		titleColor = UI_HIT;
	}
	LCD_Clear(UI_BG);
	drawTitleWithUnderline(title, GAME_OVER_TITLE_Y, titleColor);
	LCD_Draw_Circle_Fill(LCD_WIDTH / 2, GAME_OVER_TITLE_Y + GAME_OVER_MARKER_DROP, GAME_OVER_MARKER_R, titleColor);
	Display_DrawString(centerTextX(0, LCD_WIDTH, "TAP TO CONTINUE"), GAME_OVER_HINT_Y, "TAP TO CONTINUE", UI_TEXT_DIM);
}

void Display_RenderTransitionScreen(const char *message)
{
	LCD_Clear(UI_BG);
	drawTitleWithUnderline(message, TRANSITION_TITLE_Y, UI_ACCENT);
	Display_DrawString(centerTextX(0, LCD_WIDTH, "PRESS BTN TO START"), TRANSITION_HINT_Y, "PRESS BTN TO START", UI_TEXT_DIM);
}

void Display_RenderStateForTarget(GameState target)
{
	switch (target) {
		case STATE_P2_PLACEMENT: Display_RenderPlacementScreen(2); break;
		case STATE_P1_ATTACK: Display_RenderAttackScreen(1); break;
		case STATE_P2_ATTACK: Display_RenderAttackScreen(2); break;
		default: Display_RenderHomeScreen(); break;
	}
}
