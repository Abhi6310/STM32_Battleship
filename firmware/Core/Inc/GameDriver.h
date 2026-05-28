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

void GameDriver_Init(void);
void GameDriver_HandleTouch(uint16_t x, uint16_t y);
void GameDriver_HandleButtonTick(void);
void GameDriver_HandleTimerTick(void);
void GameDriver_ResetGame(void);
GameState GameDriver_GetState(void);

#endif /* INC_GAMEDRIVER_H_ */
