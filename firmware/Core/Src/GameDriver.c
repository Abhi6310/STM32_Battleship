#include "GameDriver.h"
#include "DisplayDriver.h"

#define TOUCH_IN_RECT(x, y, bx, by, bw, bh) \
    ((uint16_t)(x) >= (uint16_t)(bx) && \
     (uint16_t)(x) < (uint16_t)((bx) + (bw)) && \
     (uint16_t)(y) >= (uint16_t)(by) && \
     (uint16_t)(y) < (uint16_t)((by) + (bh)))

static GameState currentGameState = STATE_HOME_SCREEN;

static Grid playerGrid;
static Grid aiGrid;
static Ship playerShips[NUM_SHIPS];
static Ship aiShips[NUM_SHIPS];
static Ship g_placingShip;
static uint8_t g_placingShipIndex;
static uint8_t g_shipsPlaced;
static uint8_t g_isMultiplayer;
static uint16_t buttonHeldTicks;

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
    for (uint8_t r = 0; r < GRID_SIZE; r++)
        for (uint8_t c = 0; c < GRID_SIZE; c++)
            playerGrid.cells[r][c] = CELL_EMPTY;

    for (uint8_t r = 0; r < GRID_SIZE; r++)
        for (uint8_t c = 0; c < GRID_SIZE; c++)
            aiGrid.cells[r][c] = CELL_EMPTY;

    playerShips[0] = (Ship){ SHIP_DESTROYER, DESTROYER_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    playerShips[1] = (Ship){ SHIP_SUBMARINE, SUBMARINE_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    playerShips[2] = (Ship){ SHIP_BATTLESHIP, BATTLESHIP_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };

    aiShips[0] = (Ship){ SHIP_DESTROYER, DESTROYER_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    aiShips[1] = (Ship){ SHIP_SUBMARINE, SUBMARINE_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    aiShips[2] = (Ship){ SHIP_BATTLESHIP, BATTLESHIP_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };

    g_shipsPlaced = 0;
    g_placingShipIndex = 0;
    g_isMultiplayer = 0;
    buttonHeldTicks = 0;

    currentGameState = STATE_HOME_SCREEN;
}

void GameDriver_HandleTouch(uint16_t x, uint16_t y)
{
    switch (currentGameState)
    {
        case STATE_HOME_SCREEN:
            if (TOUCH_IN_RECT(x, y, BTN_1P_X, BTN_1P_Y, BTN_W, BTN_H))
            {
                currentGameState = STATE_P1_PLACEMENT;
            }
            else if (TOUCH_IN_RECT(x, y, BTN_2P_X, BTN_2P_Y, BTN_W, BTN_H))
            {
                g_isMultiplayer = 1;
                currentGameState = STATE_P1_PLACEMENT;
            }
            else if (TOUCH_IN_RECT(x, y, BTN_STATS_X, BTN_STATS_Y, BTN_W, BTN_H))
            {
                currentGameState = STATE_STATS_SCREEN;
            }
            break;
        default:
            break;
    }
}

void GameDriver_HandleButtonTick(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
        buttonHeldTicks++;
    }
    else
    {
        buttonHeldTicks = 0;
    }
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

Grid* GameDriver_GetPlayerGrid(void)
{
    return &playerGrid;
}

Grid* GameDriver_GetAIGrid(void)
{
    return &aiGrid;
}

Ship* GameDriver_GetPlayerShips(void)
{
    return playerShips;
}

Ship* GameDriver_GetAIShips(void)
{
    return aiShips;
}

Ship* GameDriver_GetPlacingShip(void)
{
    return &g_placingShip;
}

uint8_t GameDriver_GetPlacingShipIndex(void)
{
    return g_placingShipIndex;
}
