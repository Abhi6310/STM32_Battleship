#include "ApplicationCode.h"
#include "LCD_Driver.h"
#include "GameDriver.h"
#include "DisplayDriver.h"
#include "TouchConfig.h"

#define TOUCH_Y_INVERT_MAX 295
#define TIMER_PSC_10MS 839
#define TIMER_ARR_10MS 999
#define TOUCH_RELEASE_CONFIRM 100

static STMPE811_TouchData touchData;
volatile uint8_t timerFlag = 0;

void TIM6_DAC_IRQHandler(void)
{
	__HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);
	timerFlag = 1;
}

void ApplicationInit(void)
{
	LTCD__Init();
	LTCD_Layer_Init(0);
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetFont(&Font);
	InitializeLCDTouch();
	touchData.orientation = TOUCH_ORIENTATION;
	GameDriver_Init();
	Flash_LoadStats();
	RNG_Init();
	Button_Init();
	Timer_Init(TIMER_PSC_10MS, TIMER_ARR_10MS);
	Display_RenderHomeScreen();
}

void ApplicationRun(void)
{
	uint16_t releaseStreak = TOUCH_RELEASE_CONFIRM;

	while (1)
	{
		if (timerFlag)
		{
			timerFlag = 0;
			GameDriver_HandleButtonTick();
			GameDriver_HandleTimerTick();
		}

		STMPE811_State_t state = returnTouchStateAndLocation(&touchData);
		if (state == TOUCH_RELEASED)
		{
			if (releaseStreak < TOUCH_RELEASE_CONFIRM) releaseStreak++;
		}
		else
		{
			if (releaseStreak >= TOUCH_RELEASE_CONFIRM)
			{
				touchData.y = TOUCH_Y_INVERT_MAX - touchData.y;
				GameDriver_HandleTouch(touchData.x, touchData.y);
			}
			releaseStreak = 0;
		}
	}
}
