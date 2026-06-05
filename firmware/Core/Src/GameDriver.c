#include "GameDriver.h"
#include "DisplayDriver.h"
#include <string.h>

#define BUTTON_HOLD_TICKS_3S 300
#define PLACING_ORIENTATION_DEFAULT ORIENTATION_HORIZONTAL
#define MAX_PLACEMENT_ATTEMPTS 100
#define AI_QUEUE_MAX 16
#define AI_RESULT_DISPLAY_TICKS 100

#define STATS_FLASH_SECTOR FLASH_SECTOR_11
#define STATS_FLASH_ADDR ((uint32_t)0x081C0000)
#define STATS_MAGIC ((uint32_t)0xDEADBEEF)

TIM_HandleTypeDef htim6;
static RNG_HandleTypeDef rngHandle;
static GameStats g_stats;

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
static GameState nextTransitionTarget = STATE_HOME_SCREEN;
static uint16_t buttonHeldTicks;

static uint8_t aiTargetQueue[AI_QUEUE_MAX][2];
static uint8_t aiTargetHead;
static uint8_t aiTargetCount;
static uint8_t aiInTargetMode;
static uint8_t aiAttackFired;
static uint16_t aiResultDelay;
static AttackResult aiLastResult;

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
    nextTransitionTarget = STATE_HOME_SCREEN;
    buttonHeldTicks = 0;

    aiTargetHead = 0;
    aiTargetCount = 0;
    aiInTargetMode = 0;
    aiAttackFired = 0;
    aiResultDelay = 0;
    aiLastResult = ATTACK_MISS;

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
                Display_RenderStatsScreen();
            }
            break;

        case STATE_STATS_SCREEN:
            if (pointInRect(x, y, BTN_RESET_X, STATS_RESET_Y, BTN_W, BTN_H))
            {
                memset(&g_stats, 0, sizeof(g_stats));
                Flash_SaveStats();
                Display_RenderStatsScreen();
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
                        if (g_isMultiplayer)
                        {
                            nextTransitionTarget = STATE_P2_PLACEMENT;
                            currentGameState = STATE_TRANSITION_SCREEN;
                            Display_RenderTransitionScreen("Pass to P2");
                        }
                        else
                        {
                            currentGameState = STATE_AI_SETUP;
                            AI_PlaceShips(&aiGrid, aiShips);
                            currentGameState = STATE_P1_ATTACK;
                            Display_RenderAttackScreen(1);
                        }
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

        case STATE_P2_PLACEMENT:
            if (Touch_IsInGrid(x, y))
            {
                g_placingShip.startRow = Touch_ToGridRow(y);
                g_placingShip.startCol = Touch_ToGridCol(x);
                Display_UpdatePlacementPreview();
            }
            else if (pointInRect(x, y, BTN_PLACE_X, BTN_PLACE_Y, BTN_W, BTN_H))
            {
                if (GameDriver_IsPlacementValid(&aiGrid, &g_placingShip))
                {
                    for (uint8_t i = 0; i < g_placingShip.length; i++)
                    {
                        uint8_t r, c;
                        shipCellAt(&g_placingShip, i, &r, &c);
                        aiGrid.cells[r][c] = CELL_SHIP;
                    }
                    aiShips[g_placingShipIndex] = g_placingShip;
                    aiShips[g_placingShipIndex].isPlaced = 1;
                    g_shipsPlaced++;
                    g_placingShipIndex++;

                    if (g_shipsPlaced >= NUM_SHIPS)
                    {
                        nextTransitionTarget = STATE_P1_ATTACK;
                        currentGameState = STATE_TRANSITION_SCREEN;
                        Display_RenderTransitionScreen("Pass back to P1");
                    }
                    else
                    {
                        g_placingShip = aiShips[g_placingShipIndex];
                        g_placingShip.orientation = PLACING_ORIENTATION_DEFAULT;
                        g_placingShip.startRow = 0;
                        g_placingShip.startCol = 0;
                        Display_RenderPlacementScreen(2);
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
                        g_stats.playerWins++;
                        Flash_SaveStats();
                        currentGameState = STATE_GAME_OVER;
                        Display_RenderGameOver(1);
                    }
                }
                else
                {
                    if (g_isMultiplayer)
                    {
                        nextTransitionTarget = STATE_P2_ATTACK;
                        currentGameState = STATE_TRANSITION_SCREEN;
                        Display_RenderTransitionScreen("MISS! Pass to P2");
                    }
                    else
                    {
                        Display_ShowMissResult(1);
                        currentGameState = STATE_AI_ATTACK;
                    }
                }
            }
            break;

        case STATE_P2_ATTACK:
            if (Touch_IsInGrid(x, y))
            {
                uint8_t row = Touch_ToGridRow(y);
                uint8_t col = Touch_ToGridCol(x);
                AttackResult res = processAttack(&playerGrid, playerShips, row, col);
                if (res == ATTACK_INVALID) break;
                if (res == ATTACK_HIT || res == ATTACK_SUNK)
                {
                    Display_ShowHitResult(2);
                    if (res == ATTACK_SUNK && GameDriver_CheckWin(playerShips))
                    {
                        g_stats.aiWins++;
                        Flash_SaveStats();
                        currentGameState = STATE_GAME_OVER;
                        Display_RenderGameOver(2);
                    }
                }
                else
                {
                    nextTransitionTarget = STATE_P1_ATTACK;
                    currentGameState = STATE_TRANSITION_SCREEN;
                    Display_RenderTransitionScreen("MISS! Pass to P1");
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

GameStats* GameDriver_GetStats(void)
{
    return &g_stats;
}

void Flash_LoadStats(void)
{
    const GameStats *flashPtr = (const GameStats *)STATS_FLASH_ADDR;
    if (flashPtr->validationMagic == STATS_MAGIC)
    {
        g_stats = *flashPtr;
    }
    else
    {
        memset(&g_stats, 0, sizeof(g_stats));
    }
}

void Flash_SaveStats(void)
{
    g_stats.validationMagic = STATS_MAGIC;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks = FLASH_BANK_1;
    eraseInit.Sector = STATS_FLASH_SECTOR;
    eraseInit.NbSectors = 1;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    const uint32_t *src = (const uint32_t *)&g_stats;
    uint32_t addr = STATS_FLASH_ADDR;
    for (uint32_t i = 0; i < sizeof(GameStats) / sizeof(uint32_t); i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, src[i]);
        addr += sizeof(uint32_t);
    }

    HAL_FLASH_Lock();
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
            if (currentGameState == STATE_P1_PLACEMENT ||
                currentGameState == STATE_P2_PLACEMENT)
            {
                GameDriver_RotateSelectedShip();
                Display_UpdatePlacementPreview();
            }
            else if (currentGameState == STATE_TRANSITION_SCREEN)
            {
                if (nextTransitionTarget == STATE_P2_PLACEMENT)
                {
                    beginPlayerPlacement(2);
                }
                else
                {
                    currentGameState = nextTransitionTarget;
                    Display_RenderStateForTarget(nextTransitionTarget);
                }
            }
            else if (currentGameState == STATE_GAME_OVER)
            {
                GameDriver_ResetGame();
                Display_RenderHomeScreen();
            }
            else if (currentGameState == STATE_STATS_SCREEN)
            {
                currentGameState = STATE_HOME_SCREEN;
                Display_RenderHomeScreen();
            }
        }
        buttonHeldTicks = 0;
    }
}

void GameDriver_HandleTimerTick(void)
{
    if (currentGameState != STATE_AI_ATTACK)
    {
        aiAttackFired = 0;
        aiResultDelay = 0;
        return;
    }

    if (!aiAttackFired)
    {
        aiLastResult = AI_Attack(&playerGrid, playerShips);
        if (aiLastResult == ATTACK_HIT || aiLastResult == ATTACK_SUNK)
            Display_ShowHitResult(0);
        else
            Display_ShowMissResult(0);
        aiAttackFired = 1;
        aiResultDelay = 0;
        return;
    }

    aiResultDelay++;
    if (aiResultDelay < AI_RESULT_DISPLAY_TICKS) return;

    aiAttackFired = 0;
    aiResultDelay = 0;

    if (aiLastResult == ATTACK_MISS)
    {
        currentGameState = STATE_P1_ATTACK;
        Display_RenderAttackScreen(1);
        return;
    }

    if (GameDriver_CheckWin(playerShips))
    {
        g_stats.aiWins++;
        Flash_SaveStats();
        currentGameState = STATE_GAME_OVER;
        Display_RenderGameOver(0);
    }
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
    nextTransitionTarget = STATE_HOME_SCREEN;
    buttonHeldTicks = 0;

    aiTargetHead = 0;
    aiTargetCount = 0;
    aiInTargetMode = 0;
    aiAttackFired = 0;
    aiResultDelay = 0;
    aiLastResult = ATTACK_MISS;

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
    Grid *activeGrid = (currentGameState == STATE_P2_PLACEMENT) ? &aiGrid : &playerGrid;
    if (GameDriver_IsPlacementValid(activeGrid, &temp))
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
        g_stats.heatmap[row][col]++;

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

static uint8_t isUnshotCell(const Grid *grid, uint8_t row, uint8_t col)
{
    CellState cs = grid->cells[row][col];
    return (cs == CELL_EMPTY) || (cs == CELL_SHIP);
}

static void queueNeighborIfUnshot(const Grid *grid, int8_t r, int8_t c)
{
    if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) return;
    if (!isUnshotCell(grid, (uint8_t)r, (uint8_t)c)) return;
    if (aiTargetCount >= AI_QUEUE_MAX) return;
    uint8_t tail = (uint8_t)((aiTargetHead + aiTargetCount) % AI_QUEUE_MAX);
    aiTargetQueue[tail][0] = (uint8_t)r;
    aiTargetQueue[tail][1] = (uint8_t)c;
    aiTargetCount++;
}

AttackResult AI_Attack(Grid *grid, Ship ships[])
{
    uint8_t row = 0;
    uint8_t col = 0;
    uint8_t haveCell = 0;

    if (aiInTargetMode)
    {
        while (aiTargetCount > 0)
        {
            uint8_t tr = aiTargetQueue[aiTargetHead][0];
            uint8_t tc = aiTargetQueue[aiTargetHead][1];
            aiTargetHead = (uint8_t)((aiTargetHead + 1) % AI_QUEUE_MAX);
            aiTargetCount--;
            if (isUnshotCell(grid, tr, tc))
            {
                row = tr;
                col = tc;
                haveCell = 1;
                break;
            }
        }
        if (!haveCell) aiInTargetMode = 0;
    }

    if (!haveCell)
    {
        uint32_t randVal;
        do
        {
            HAL_RNG_GenerateRandomNumber(&rngHandle, &randVal);
            row = (uint8_t)(randVal % GRID_SIZE);
            HAL_RNG_GenerateRandomNumber(&rngHandle, &randVal);
            col = (uint8_t)(randVal % GRID_SIZE);
        } while (!isUnshotCell(grid, row, col));
    }

    AttackResult res = processAttack(grid, ships, row, col);

    if (res == ATTACK_HIT)
    {
        queueNeighborIfUnshot(grid, (int8_t)row - 1, (int8_t)col);
        queueNeighborIfUnshot(grid, (int8_t)row + 1, (int8_t)col);
        queueNeighborIfUnshot(grid, (int8_t)row, (int8_t)col - 1);
        queueNeighborIfUnshot(grid, (int8_t)row, (int8_t)col + 1);
        aiInTargetMode = (aiTargetCount > 0) ? 1 : 0;
    }
    else if (res == ATTACK_SUNK)
    {
        aiTargetHead = 0;
        aiTargetCount = 0;
        aiInTargetMode = 0;
    }

    return res;
}
