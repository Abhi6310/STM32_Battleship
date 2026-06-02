#include "ApplicationCode.h"
#include "LCD_Driver.h"
#include "GameDriver.h"
#include "DisplayDriver.h"

#define TOUCH_Y_INVERT_MAX 295

static STMPE811_TouchData touchData;

void ApplicationInit(void)
{
	LTCD__Init();
	LTCD_Layer_Init(0);
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetFont(&Font12x12);
	InitializeLCDTouch();
	touchData.orientation = STMPE811_Orientation_Portrait_2;
	GameDriver_Init();
	Display_RenderHomeScreen();
}

void ApplicationRun(void)
{
	while (1)
	{
		if (returnTouchStateAndLocation(&touchData) == STMPE811_State_Pressed)
		{
			touchData.y = TOUCH_Y_INVERT_MAX - touchData.y;
			if (touchData.pressed == STMPE811_State_Pressed &&
				touchData.last_pressed == STMPE811_State_Released)
			{
				GameDriver_HandleTouch(touchData.x, touchData.y);
			}
		}
	}
}
