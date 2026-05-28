/*
 * ApplicationCode.c
 */

#include "ApplicationCode.h"
#include "LCD_Driver.h"

extern void initialise_monitor_handles(void);

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData touchData;
#endif

void ApplicationInit(void)
{
	initialise_monitor_handles();
	LTCD__Init();
	LTCD_Layer_Init(0);
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetFont(&Font12x12);

#if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();
	touchData.orientation = STMPE811_Orientation_Portrait_2;
#endif
}

void LCD_Visual_Demo(void)
{
	/* intentionally empty — main.c calls this; game init happens in ApplicationInit */
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	/* game loop entry point — implementation in GameDriver */
	while (1) { }
}
#endif
