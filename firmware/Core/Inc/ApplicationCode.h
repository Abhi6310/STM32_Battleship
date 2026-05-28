#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

#include "stm32f4xx_hal.h"

void ApplicationInit(void);
void LCD_Visual_Demo(void);

#if (COMPILE_TOUCH_FUNCTIONS == 1) && (COMPILE_TOUCH_INTERRUPT_SUPPORT == 0)
void LCD_Touch_Polling_Demo(void);
#endif

#endif /* INC_APPLICATIONCODE_H_ */
