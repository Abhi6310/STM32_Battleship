#include "GameDriver.h"
#include "DisplayDriver.h"
#include <string.h>

#define BUTTON_HOLD_TICKS_3S 300
#define PLACING_ORIENTATION_DEFAULT ORIENTATION_HORIZONTAL
#define MAX_PLACEMENT_ATTEMPTS 100

TIM_HandleTypeDef htim6;
static RNG_HandleTypeDef rngHandle;

static const Ship kShipTemplate[NUM_SHIPS] = {
    { SHIP_DESTROYER, DESTROYER_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 },
    { SHIP_SUBMARINE, SUBMARINE_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 },
    { SHIP_BATTLESHIP, BATTLESHIP_LENGTH, 0, 0, ORIENTATION_HORIZONTAL, 0, 1 },
};

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

static inline uint8_t pointInRect(uint16_t x, uint16_t y,
                                  uint16_t rx, uint16_t ry,
                                  uint16_t rw, uint16_t rh)
{
    return (x >= rx) && (x < rx + rw) && (y >= ry) && (y < ry + rh);
}

static inline void shipCellAt(const Ship *s, uint8_t i, uint8_t *row, uint8_t *col)
{
    *row = s->startRow + ((s->orientation == ORIENTATION_VERTICAL) ? i : 0);
    *col = s->startCol + ((s->orientation == ORIENTATION_HORIZONTAL) ? i : 0);
}

void GameDriver_GetShipCell(const Ship *ship, uint8_t i, uint8_t *row, uint8_t *col)
{
    shipCellAt(ship, i, row, col);
}

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
    memset(&playerGrid, 0, sizeof(playerGrid));
    memset(&aiGrid, 0, sizeof(aiGrid));
    memcpy(playerShips, kShipTemplate, sizeof(kShipTemplate));
    memcpy(aiShips, kShipTemplate, sizeof(kShipTemplate));

    g_shipsPlaced = 0;
    g_placingShipIndex = 0;
    g_isMultiplayer = 0;
    buttonHeldTicks = 0;

    currentGameState = STATE_HOME_SCREEN;
}

static void beginPlayerPlacement(uint8_t player)
{
    g_shipsPlaced = 0;
    g_placingShipIndex = 0;
    g_placingShip = playerShips[0];
    g_placingShip.orientation = PLACING_ORIENTATION_DEFAULT;
    g_placingShip.startRow = 0;
    g_placingShip.startCol = 0;
    currentGameState = (player == 2) ? STATE_P2_PLACEMENT : STATE_P1_PLACEMENT;
    Display_RenderPlacementScreen(player);
}

void GameDriver_HandleTouch(uint16_t x, uint16_t y)
{
    switch (currentGameState)
    {
        case STATE_HOME_SCREEN:
            if (pointInRect(x, y, HOME_BTN_X, BTN_SINGLE_Y, HOME_BTN_W, HOME_BTN_H))
            {
                beginPlayerPlacement(1);
            }
            else if (pointInRect(x, y, HOME_BTN_X, BTN_MULTI_Y, HOME_BTN_W, HOME_BTN_H))
            {
                g_isMultiplayer = 1;
                beginPlayerPlacement(1);
            }
            else if (pointInRect(x, y, HOME_BTN_X, BTN_STATS_Y, HOME_BTN_W, HOME_BTN_H))
            {
                currentGameState = STATE_STATS_SCREEN;
                Display_RenderStatsPlaceholder();
            }
            break;

        case STATE_P1_PLACEMENT:
            if (Touch_IsInGrid(x, y))
            {
                g_placingShip.startRow = Touch_ToGridRow(y);
                g_placingShip.startCol = Touch_ToGridCol(x);
                Display_UpdatePlacementPreview();
            }
            else if (pointInRect(x, y, BTN_PLACE_X, BTN_PLACE_Y, BTN_W, BTN_H))
            {
                if (GameDriver_IsPlacementValid(&playerGrid, &g_placingShip))
                {
                    for (uint8_t i = 0; i < g_placingShip.length; i++)
                    {
                        uint8_t r, c;
                        shipCellAt(&g_placingShip, i, &r, &c);
                        playerGrid.cells[r][c] = CELL_SHIP;
                    }
                    playerShips[g_placingShipIndex] = g_placingShip;
                    playerShips[g_placingShipIndex].isPlaced = 1;
                    g_shipsPlaced++;
                    g_placingShipIndex++;

                    if (g_shipsPlaced >= NUM_SHIPS)
                    {
                        currentGameState = STATE_AI_SETUP;
                        AI_PlaceShips(&aiGrid, aiShips);
                        currentGameState = STATE_P1_ATTACK;
                        Display_RenderAttackScreen(1);
                    }
                    else
                    {
                        g_placingShip = playerShips[g_placingShipIndex];
                        g_placingShip.orientation = PLACING_ORIENTATION_DEFAULT;
                        g_placingShip.startRow = 0;
                        g_placingShip.startCol = 0;
                        Display_RenderPlacementScreen(1);
                    }
                }
            }
            break;

        case STATE_P1_ATTACK:
            if (Touch_IsInGrid(x, y))
            {
                uint8_t row = Touch_ToGridRow(y);
                uint8_t col = Touch_ToGridCol(x);
                AttackResult res = processAttack(&aiGrid, aiShips, row, col);
                if (res == ATTACK_INVALID) break;
                if (res == ATTACK_HIT || res == ATTACK_SUNK)
                {
                    Display_ShowHitResult(1);
                    if (res == ATTACK_SUNK && GameDriver_CheckWin(aiShips))
                    {
                        currentGameState = STATE_GAME_OVER;
                        Display_RenderGameOver(1);
                    }
                }
                else
                {
                    Display_ShowMissResult(1);
                }
            }
            break;

        case STATE_GAME_OVER:
            GameDriver_ResetGame();
            Display_RenderHomeScreen();
            break;

        default:
            break;
    }
}

void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStructure.Pin = GPIO_PIN_0;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
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

void RNG_Init(void)
{
    __HAL_RCC_RNG_CLK_ENABLE();
    rngHandle.Instance = RNG;
    HAL_RNG_Init(&rngHandle);
}

void AI_PlaceShips(Grid *grid, Ship ships[])
{
    for (uint8_t i = 0; i < NUM_SHIPS; i++)
    {
        uint32_t randVal;
        for (uint16_t attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; attempt++)
        {
            HAL_RNG_GenerateRandomNumber(&rngHandle, &randVal);
            ships[i].startRow = randVal % GRID_SIZE;
            HAL_RNG_GenerateRandomNumber(&rngHandle, &randVal);
            ships[i].startCol = randVal % GRID_SIZE;
            HAL_RNG_GenerateRandomNumber(&rngHandle, &randVal);
            ships[i].orientation = (randVal & 1) ? ORIENTATION_VERTICAL : ORIENTATION_HORIZONTAL;
            if (GameDriver_IsPlacementValid(grid, &ships[i])) break;
        }
        for (uint8_t j = 0; j < ships[i].length; j++)
        {
            uint8_t r, c;
            shipCellAt(&ships[i], j, &r, &c);
            grid->cells[r][c] = CELL_SHIP;
        }
        ships[i].isPlaced = 1;
    }
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
        if (buttonHeldTicks > 0 && buttonHeldTicks < BUTTON_HOLD_TICKS_3S)
        {
            if (currentGameState == STATE_P1_PLACEMENT)
            {
                GameDriver_RotateSelectedShip();
                Display_UpdatePlacementPreview();
            }
            else if (currentGameState == STATE_GAME_OVER)
            {
                GameDriver_ResetGame();
                Display_RenderHomeScreen();
            }
        }
        buttonHeldTicks = 0;
    }
}

void GameDriver_HandleTimerTick(void)
{
}

void GameDriver_ResetGame(void)
{
    memset(&playerGrid, 0, sizeof(playerGrid));
    memset(&aiGrid, 0, sizeof(aiGrid));
    memcpy(playerShips, kShipTemplate, sizeof(kShipTemplate));
    memcpy(aiShips, kShipTemplate, sizeof(kShipTemplate));

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

uint8_t GameDriver_IsPlacementValid(Grid *grid, Ship *ship)
{
    for (uint8_t i = 0; i < ship->length; i++)
    {
        uint8_t r, c;
        shipCellAt(ship, i, &r, &c);
        if (r >= GRID_SIZE || c >= GRID_SIZE) return 0;
        if (grid->cells[r][c] != CELL_EMPTY) return 0;
    }
    return 1;
}

void GameDriver_RotateSelectedShip(void)
{
    Ship temp = g_placingShip;
    temp.orientation = (temp.orientation == ORIENTATION_HORIZONTAL) ? ORIENTATION_VERTICAL : ORIENTATION_HORIZONTAL;
    if (GameDriver_IsPlacementValid(&playerGrid, &temp))
    {
        g_placingShip.orientation = temp.orientation;
    }
}

static uint8_t isShipAtCell(const Ship *ship, uint8_t row, uint8_t col)
{
    for (uint8_t i = 0; i < ship->length; i++)
    {
        uint8_t r, c;
        shipCellAt(ship, i, &r, &c);
        if (r == row && c == col) return 1;
    }
    return 0;
}

static AttackResult processAttack(Grid *grid, Ship ships[], uint8_t row, uint8_t col)
{
    CellState existing = grid->cells[row][col];
    if (existing == CELL_HIT || existing == CELL_MISS || existing == CELL_SUNK)
        return ATTACK_INVALID;

    for (uint8_t i = 0; i < NUM_SHIPS; i++)
    {
        if (!isShipAtCell(&ships[i], row, col)) continue;

        grid->cells[row][col] = CELL_HIT;
        ships[i].hitCount++;

        if (ships[i].hitCount >= ships[i].length)
        {
            for (uint8_t j = 0; j < ships[i].length; j++)
            {
                uint8_t r, c;
                shipCellAt(&ships[i], j, &r, &c);
                grid->cells[r][c] = CELL_SUNK;
            }
            return ATTACK_SUNK;
        }
        return ATTACK_HIT;
    }

    grid->cells[row][col] = CELL_MISS;
    return ATTACK_MISS;
}

uint8_t GameDriver_CheckWin(Ship ships[])
{
    for (uint8_t i = 0; i < NUM_SHIPS; i++)
    {
        if (ships[i].hitCount < ships[i].length) return 0;
    }
    return 1;
}
