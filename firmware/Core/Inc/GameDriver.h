#ifndef INC_GAMEDRIVER_H_
#define INC_GAMEDRIVER_H_

#include "stm32f4xx_hal.h"

typedef enum {
    STATE_HOME_SCREEN,
    STATE_P1_PLACEMENT,
    STATE_P2_PLACEMENT,
    STATE_AI_SETUP,
    STATE_P1_ATTACK,
    STATE_AI_ATTACK,
    STATE_P2_ATTACK,
    STATE_TRANSITION_SCREEN,
    STATE_GAME_OVER,
    STATE_STATS_SCREEN,
} GameState;

#define GRID_SIZE 7
#define NUM_SHIPS 3
#define DESTROYER_LENGTH 2
#define SUBMARINE_LENGTH 3
#define BATTLESHIP_LENGTH 4

typedef enum { CELL_EMPTY, CELL_SHIP, CELL_HIT, CELL_MISS, CELL_SUNK } CellState;

typedef struct {
    CellState cells[GRID_SIZE][GRID_SIZE];
} Grid;

typedef enum { SHIP_DESTROYER, SHIP_SUBMARINE, SHIP_BATTLESHIP } ShipType;
typedef enum { ORIENTATION_HORIZONTAL, ORIENTATION_VERTICAL } ShipOrientation;

typedef struct {
    ShipType type;
    uint8_t length;
    uint8_t startRow;
    uint8_t startCol;
    ShipOrientation orientation;
    uint8_t hitCount;
    uint8_t isPlaced;
} Ship;

typedef enum {
    ATTACK_HIT,
    ATTACK_MISS,
    ATTACK_SUNK,
    ATTACK_INVALID
} AttackResult;

typedef struct {
    uint32_t playerWins;
    uint32_t aiWins;
    uint32_t heatmap[GRID_SIZE][GRID_SIZE];
    uint32_t validationMagic;
} GameStats;

uint8_t Touch_IsInGrid(uint16_t px, uint16_t py);
uint8_t Touch_ToGridRow(uint16_t py);
uint8_t Touch_ToGridCol(uint16_t px);

void RNG_Init(void);
void AI_PlaceShips(Grid *grid, Ship ships[]);
AttackResult AI_Attack(Grid *grid, Ship ships[]);
uint8_t GameDriver_CheckWin(Ship ships[]);

extern TIM_HandleTypeDef htim6;

void Timer_Init(uint16_t psc, uint16_t arr);
void Button_Init(void);
void GameDriver_Init(void);
void GameDriver_HandleTouch(uint16_t x, uint16_t y);
void GameDriver_HandleButtonTick(void);
void GameDriver_HandleTimerTick(void);
void GameDriver_ResetGame(void);
GameState GameDriver_GetState(void);

Grid* GameDriver_GetPlayerGrid(void);
Grid* GameDriver_GetAIGrid(void);
Ship* GameDriver_GetPlayerShips(void);
Ship* GameDriver_GetAIShips(void);
Ship* GameDriver_GetPlacingShip(void);
uint8_t GameDriver_GetPlacingShipIndex(void);

uint8_t GameDriver_IsPlacementValid(Grid *grid, Ship *ship);
void GameDriver_RotateSelectedShip(void);
void GameDriver_GetShipCell(const Ship *ship, uint8_t i, uint8_t *row, uint8_t *col);

GameStats* GameDriver_GetStats(void);
void Flash_LoadStats(void);
void Flash_SaveStats(void);

#endif /* INC_GAMEDRIVER_H_ */
