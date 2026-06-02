#include "GameDriver.h"
#include "DisplayDriver.h"

#define TOUCH_IN_RECT(x, y, bx, by, bw, bh) \
    ((uint16_t)(x) >= (uint16_t)(bx) && \
     (uint16_t)(x) < (uint16_t)((bx) + (bw)) && \
     (uint16_t)(y) >= (uint16_t)(by) && \
     (uint16_t)(y) < (uint16_t)((by) + (bh)))

#define BUTTON_HOLD_TICKS_3S 300

TIM_HandleTypeDef htim6;

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
            if (TOUCH_IN_RECT(x, y, HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H))
            {
                currentGameState = STATE_P1_PLACEMENT;
                Display_RenderPlacementPlaceholder(0);
            }
            else if (TOUCH_IN_RECT(x, y, HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H))
            {
                g_isMultiplayer = 1;
                currentGameState = STATE_P1_PLACEMENT;
                Display_RenderPlacementPlaceholder(1);
            }
            else if (TOUCH_IN_RECT(x, y, HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H))
            {
                currentGameState = STATE_STATS_SCREEN;
                Display_RenderStatsPlaceholder();
            }
            break;
        default:
            break;
    }
}

void Timer_Init(uint16_t psc, uint16_t arr)
{
    __HAL_RCC_TIM6_CLK_ENABLE();
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = psc;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = arr;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim6);
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    HAL_TIM_Base_Start_IT(&htim6);
}

void GameDriver_HandleButtonTick(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
    {
        buttonHeldTicks++;
        if (buttonHeldTicks >= BUTTON_HOLD_TICKS_3S &&
            currentGameState != STATE_HOME_SCREEN)
        {
            buttonHeldTicks = 0;
            GameDriver_ResetGame();
            Display_RenderHomeScreen();
        }
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
    for (uint8_t r = 0; r < GRID_SIZE; r++)
        for (uint8_t c = 0; c < GRID_SIZE; c++)
            playerGrid.cells[r][c] = CELL_EMPTY;

    for (uint8_t r = 0; r < GRID_SIZE; r++)
        for (uint8_t c = 0; c < GRID_SIZE; c++)
            aiGrid.cells[r][c] = CELL_EMPTY;

    playerShips[0] = (Ship){ SHIP_DESTROYER, DESTROYER_LENGTH,  0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    playerShips[1] = (Ship){ SHIP_SUBMARINE, SUBMARINE_LENGTH,  0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    playerShips[2] = (Ship){ SHIP_BATTLESHIP, BATTLESHIP_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };

    aiShips[0] = (Ship){ SHIP_DESTROYER,  DESTROYER_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    aiShips[1] = (Ship){ SHIP_SUBMARINE,  SUBMARINE_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };
    aiShips[2] = (Ship){ SHIP_BATTLESHIP, BATTLESHIP_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 };

    g_shipsPlaced = 0;
    g_placingShipIndex = 0;
    g_isMultiplayer = 0;
    buttonHeldTicks = 0;

    currentGameState = STATE_HOME_SCREEN;
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
