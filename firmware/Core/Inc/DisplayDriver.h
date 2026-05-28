#ifndef INC_DISPLAYDRIVER_H_
#define INC_DISPLAYDRIVER_H_

#include "GameDriver.h"
#include "stm32f4xx_hal.h"

void Display_RenderHomeScreen(void);
void Display_RenderPlacementScreen(void);
void Display_RenderAttackScreen(void);
void Display_RenderTransitionScreen(const char *msg);
void Display_RenderGameOverScreen(uint8_t playerWon);
void Display_RenderStatsScreen(void);
void Display_ShowHitResult(uint8_t attackerNum);
void Display_ShowMissPopup(void);
void Display_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void Display_DrawString(uint16_t x, uint16_t y, const char *str, uint32_t color);

#endif /* INC_DISPLAYDRIVER_H_ */
