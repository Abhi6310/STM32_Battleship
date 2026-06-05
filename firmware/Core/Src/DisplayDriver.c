#include "DisplayDriver.h"
#include "LCD_Driver.h"
#include <string.h>

static uint32_t getCellDisplayColor(CellState cell, uint8_t isAttackView)
{
    switch (cell)
    {
        case CELL_EMPTY: return LCD_COLOR_GRAY;
        case CELL_SHIP: return isAttackView ? LCD_COLOR_GRAY : LCD_COLOR_LIGHTBLUE;
        case CELL_HIT: return LCD_COLOR_RED;
        case CELL_MISS: return LCD_COLOR_DARKBLUE;
        case CELL_SUNK: return LCD_COLOR_DARKRED;
        default: return LCD_COLOR_GRAY;
    }
}

static uint16_t centerTextX(uint16_t boxX, uint16_t boxW, const char *str)
{
    uint16_t textPx = (uint16_t)strlen(str) * Font.Width;
    return boxX + (boxW - textPx) / 2;
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
		curX += Font.Width;
		str++;
	}
}

void Display_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint32_t color)
{
	LCD_SetFont(&FontLarge);
	LCD_SetTextColor(color);
	uint16_t curX = x;
	while (*str)
	{
		LCD_DisplayChar(curX, y, (uint8_t)*str);
		curX += FontLarge.Width;
		str++;
	}
	LCD_SetFont(&Font);
}

void Display_RenderHomeScreen(void)
{
	LCD_Clear(LCD_COLOR_BLACK);
	Display_DrawStringLarge(HOME_TITLE_X, HOME_TITLE_Y, "BATTLESHIP", LCD_COLOR_CYAN);

	Display_FillRect(HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_BLUE);
	Display_DrawRect(HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(centerTextX(HOME_BTN_X, HOME_BTN_W, "Singleplayer"),
	                   BTN_SINGLE_Y + BTN_TEXT_Y_OFFSET, "Singleplayer", LCD_COLOR_WHITE);

	Display_FillRect(HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_DARKGREEN);
	Display_DrawRect(HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(centerTextX(HOME_BTN_X, HOME_BTN_W, "Multiplayer"),
	                   BTN_MULTI_Y + BTN_TEXT_Y_OFFSET, "Multiplayer", LCD_COLOR_WHITE);

	Display_FillRect(HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_ORANGE);
	Display_DrawRect(HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(centerTextX(HOME_BTN_X, HOME_BTN_W, "Stats"),
	                   BTN_STATS_Y + BTN_TEXT_Y_OFFSET, "Stats", LCD_COLOR_WHITE);
}

void Display_RenderPlacementScreen(uint8_t player)
{
	LCD_Clear(LCD_COLOR_BLACK);
	Display_DrawString(PLACEMENT_HEADER_X, PLACEMENT_HEADER_Y,
	                   (player == 2) ? "P2: Place your ships" : "P1: Place your ships",
	                   LCD_COLOR_WHITE);

	Grid *activeGrid = (player == 2) ? GameDriver_GetAIGrid() : GameDriver_GetPlayerGrid();
	Display_RenderGrid(activeGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, 0);

	Display_FillRect(BTN_PLACE_X, BTN_PLACE_Y, BTN_W, BTN_H, LCD_COLOR_BLUE);
	Display_DrawRect(BTN_PLACE_X, BTN_PLACE_Y, BTN_W, BTN_H, LCD_COLOR_WHITE);
	Display_DrawString(centerTextX(BTN_PLACE_X, BTN_W, "PLACE"),
	                   BTN_PLACE_Y + BTN_TEXT_Y_OFFSET, "PLACE", LCD_COLOR_WHITE);

	uint8_t shipIdx = GameDriver_GetPlacingShipIndex();
	if (shipIdx < NUM_SHIPS)
	{
		const char *names[NUM_SHIPS] = { "Destroyer(2)", "Submarine(3)", "Battleship(4)" };
		Display_DrawString(SHIP_NAME_X, SHIP_NAME_Y, names[shipIdx], LCD_COLOR_YELLOW);
		Display_DrawString(HINT_TEXT_X, HINT_TEXT_Y, "Btn=rotate Tap=select", LCD_COLOR_GRAY);
	}
}

void Display_UpdatePlacementPreview(void)
{
	uint8_t isP2 = (GameDriver_GetState() == STATE_P2_PLACEMENT);
	Grid *activeGrid = isP2 ? GameDriver_GetAIGrid() : GameDriver_GetPlayerGrid();
	Ship *pendingShip = GameDriver_GetPlacingShip();

	Display_RenderGrid(activeGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, 0);

	uint32_t previewColor = GameDriver_IsPlacementValid(activeGrid, pendingShip) ? LCD_COLOR_CYAN : LCD_COLOR_RED;
	for (uint8_t i = 0; i < pendingShip->length; i++)
	{
		uint8_t r, c;
		GameDriver_GetShipCell(pendingShip, i, &r, &c);
		if (r < GRID_SIZE && c < GRID_SIZE)
		{
			uint16_t px = GRID_ORIGIN_X + c * GRID_CELL_SIZE;
			uint16_t py = GRID_ORIGIN_Y + r * GRID_CELL_SIZE;
			Display_FillRect(px + 1, py + 1, GRID_CELL_SIZE - 2, GRID_CELL_SIZE - 2, previewColor);
		}
	}
}

#define DEC_DIGITS_U32 10

static void u32ToStr(uint32_t num, char *buf)
{
    char tmp[DEC_DIGITS_U32];
    uint8_t n = 0;
    if (num == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (num > 0 && n < DEC_DIGITS_U32)
    {
        tmp[n++] = (char)('0' + (num % 10));
        num /= 10;
    }
    for (uint8_t i = 0; i < n; i++)
    {
        buf[i] = tmp[n - 1 - i];
    }
    buf[n] = '\0';
}

static uint32_t heatmapColor(uint32_t hits)
{
    if (hits == 0) return LCD_COLOR_GRAY;
    if (hits <= HEATMAP_LOW_MAX) return LCD_COLOR_BLUE;
    if (hits <= HEATMAP_MID_MAX) return LCD_COLOR_YELLOW;
    return LCD_COLOR_RED;
}

static void drawWinLine(uint16_t y, const char *label, uint32_t value)
{
    char numBuf[11];
    char line[24];
    u32ToStr(value, numBuf);

    uint8_t i = 0;
    while (label[i] != '\0' && i < (uint8_t)(sizeof(line) - 1))
    {
        line[i] = label[i];
        i++;
    }
    uint8_t j = 0;
    while (numBuf[j] != '\0' && i < (uint8_t)(sizeof(line) - 1))
    {
        line[i++] = numBuf[j++];
    }
    line[i] = '\0';

    Display_DrawString(STATS_WIN_LABEL_X, y, line, LCD_COLOR_WHITE);
}

void Display_RenderStatsScreen(void)
{
    GameStats *stats = GameDriver_GetStats();

    LCD_Clear(LCD_COLOR_BLACK);
    Display_DrawStringLarge(STATS_TITLE_X, STATS_TITLE_Y, "STATS", LCD_COLOR_WHITE);

    drawWinLine(STATS_WIN_P1_Y, "P1 WINS: ", stats->playerWins);
    drawWinLine(STATS_WIN_P2_Y, "P2/AI: ", stats->aiWins);

    for (uint8_t row = 0; row < GRID_SIZE; row++)
    {
        for (uint8_t col = 0; col < GRID_SIZE; col++)
        {
            uint16_t px = STATS_HEATMAP_ORIGIN_X + col * STATS_HEATMAP_CELL;
            uint16_t py = STATS_HEATMAP_ORIGIN_Y + row * STATS_HEATMAP_CELL;
            Display_FillRect(px + 1, py + 1, STATS_HEATMAP_CELL - 2, STATS_HEATMAP_CELL - 2,
                             heatmapColor(stats->heatmap[row][col]));
        }
    }

    Display_FillRect(BTN_RESET_X, STATS_RESET_Y, BTN_W, BTN_H, LCD_COLOR_DARKRED);
    Display_DrawString(BTN_RESET_X + (BTN_W - (uint16_t)strlen("RESET") * Font.Width) / 2,
                       STATS_RESET_Y + BTN_TEXT_Y_OFFSET, "RESET", LCD_COLOR_WHITE);

    Display_DrawString(STATS_HINT_X, STATS_HINT_Y, "Press button for home", LCD_COLOR_WHITE);
}

static void renderShipStatusBar(uint8_t player)
{
	Ship *ownShips = (player == 2) ? GameDriver_GetAIShips() : GameDriver_GetPlayerShips();
	static const char *labels[NUM_SHIPS] = { "DD", "SUB", "BB" };
	uint16_t curX = STATUS_BAR_X;
	for (uint8_t i = 0; i < NUM_SHIPS; i++)
	{
		uint32_t color;
		if (ownShips[i].hitCount == 0) color = LCD_COLOR_GREEN;
		else if (ownShips[i].hitCount >= ownShips[i].length) color = LCD_COLOR_DARKRED;
		else color = LCD_COLOR_YELLOW;
		Display_DrawString(curX, STATUS_BAR_Y, labels[i], color);
		curX += (uint16_t)strlen(labels[i]) * Font.Width + Font.Width;
	}
}

void Display_RenderAttackScreen(uint8_t player)
{
	LCD_Clear(LCD_COLOR_BLACK);
	const char *header;
	Grid *targetGrid;
	uint8_t isAttackView;

	if (player == 0)
	{
		header = "AI Turn";
		targetGrid = GameDriver_GetPlayerGrid();
		isAttackView = 0;
	}
	else if (player == 2)
	{
		header = "P2: Tap to attack";
		targetGrid = GameDriver_GetPlayerGrid();
		isAttackView = 1;
	}
	else
	{
		header = "P1: Tap to attack";
		targetGrid = GameDriver_GetAIGrid();
		isAttackView = 1;
	}

	Display_DrawString(ATTACK_HEADER_X, ATTACK_HEADER_Y, header, LCD_COLOR_WHITE);
	Display_RenderGrid(targetGrid, GRID_ORIGIN_X, GRID_ORIGIN_Y, isAttackView);
	renderShipStatusBar(player);
}

void Display_ShowHitResult(uint8_t player)
{
	Display_RenderAttackScreen(player);
	Display_DrawStringLarge(RESULT_OVERLAY_X, RESULT_OVERLAY_Y, "HIT!", LCD_COLOR_RED);
}

void Display_ShowMissResult(uint8_t player)
{
	Display_RenderAttackScreen(player);
	Display_DrawStringLarge(RESULT_OVERLAY_X, RESULT_OVERLAY_Y, "MISS", LCD_COLOR_DARKBLUE);
}

void Display_RenderGameOver(uint8_t winner)
{
	const char *title = (winner == 1) ? "P1 WINS" : (winner == 2) ? "P2 WINS" : "AI WINS";
	LCD_Clear(LCD_COLOR_BLACK);
	Display_DrawStringLarge(GAME_OVER_TITLE_X, GAME_OVER_TITLE_Y, title, LCD_COLOR_CYAN);
	Display_DrawString(GAME_OVER_HINT_X, GAME_OVER_HINT_Y, "Tap to continue", LCD_COLOR_WHITE);
}

void Display_RenderTransitionScreen(const char *message)
{
	LCD_Clear(LCD_COLOR_DARKBLUE);
	Display_DrawStringLarge(TRANSITION_TITLE_X, TRANSITION_TITLE_Y, message, LCD_COLOR_WHITE);
	Display_DrawString(TRANSITION_HINT_X, TRANSITION_HINT_Y, "Press button when ready", LCD_COLOR_WHITE);
}

void Display_RenderStateForTarget(GameState target)
{
	switch (target)
	{
		case STATE_P2_PLACEMENT: Display_RenderPlacementScreen(2); break;
		case STATE_P1_ATTACK: Display_RenderAttackScreen(1); break;
		case STATE_P2_ATTACK: Display_RenderAttackScreen(2); break;
		default: Display_RenderHomeScreen(); break;
	}
}
