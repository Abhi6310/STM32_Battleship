#include "GameDriver.h"
#include "DisplayDriver.h"

static GameState currentGameState = STATE_HOME_SCREEN;

uint8_t Touch_IsInGrid(uint16_t px, uint16_t py)
{
    return (px >= GRID_ORIGIN_X) && (px < GRID_ORIGIN_X + GRID_PIXEL_WIDTH) &&
           (py >= GRID_ORIGIN_Y) && (py < GRID_ORIGIN_Y + GRID_PIXEL_WIDTH);
}

uint8_t Touch_ToGridRow(uint16_t py)
{
    return (py - GRID_ORIGIN_Y) / GRID_CELL_SIZE;
}

uint8_t Touch_ToGridCol(uint16_t px)
{
    return (px - GRID_ORIGIN_X) / GRID_CELL_SIZE;
}

void GameDriver_Init(void)
{
    currentGameState = STATE_HOME_SCREEN;
}

void GameDriver_HandleTouch(uint16_t x, uint16_t y)
{
    (void)x;
    (void)y;
}

void GameDriver_HandleButtonTick(void)
{
}

void GameDriver_HandleTimerTick(void)
{
}

void GameDriver_ResetGame(void)
{
}

GameState GameDriver_GetState(void)
{
    return currentGameState;
}
