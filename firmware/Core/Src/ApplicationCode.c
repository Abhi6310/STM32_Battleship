#include "ApplicationCode.h"
#include "LCD_Driver.h"

static STMPE811_TouchData touchData;

void ApplicationInit(void)
{
	LTCD__Init();
	LTCD_Layer_Init(0);
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetFont(&Font12x12);
	InitializeLCDTouch();
	touchData.orientation = STMPE811_Orientation_Portrait_2;
}

void ApplicationRun(void)
{
}
