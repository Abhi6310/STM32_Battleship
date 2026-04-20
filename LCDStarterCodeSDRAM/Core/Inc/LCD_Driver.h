/*
 * LCD_Driver.h
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#ifndef INC_LCD_DRIVER_H_
#define INC_LCD_DRIVER_H_

#include "stm32f4xx_hal.h"
#include "ili9341.h"
#include "fonts.h"
#include "stmpe811.h"
#include "sdram.h"

#define COMPILE_TOUCH_FUNCTIONS COMPILE_TOUCH
#define TOUCH_INTERRUPT_ENABLED COMPILE_TOUCH_INTERRUPT_SUPPORT

#define LCD_FRAME_BUFFER       ((uint32_t)0xD0000000)

/* LCD TEMPALTE COLOR
The color format for the Display is chosen as RGB888. 8 bits or 255 values for R,G,B. Standard RGB color. */
#define LCD_COLOR_BLUE          0x0000FF
#define LCD_COLOR_GREEN         0x00FF00
#define LCD_COLOR_RED           0xFF0000
#define LCD_COLOR_CYAN          0x00FFFF
#define LCD_COLOR_MAGENTA       0xFF00FF
#define LCD_COLOR_YELLOW        0xFFFF00
#define LCD_COLOR_LIGHTBLUE     0x8080FF
#define LCD_COLOR_LIGHTGREEN    0x80FF80
#define LCD_COLOR_LIGHTRED      0xFF8080
#define LCD_COLOR_LIGHTCYAN     0x80FFFF
#define LCD_COLOR_LIGHTMAGENTA  0xFF80FF
#define LCD_COLOR_LIGHTYELLOW   0xFFFF80
#define LCD_COLOR_DARKBLUE      0x000080
#define LCD_COLOR_DARKGREEN     0x008000
#define LCD_COLOR_DARKRED       0x800000
#define LCD_COLOR_DARKCYAN      0x008080
#define LCD_COLOR_DARKMAGENTA   0x800080
#define LCD_COLOR_DARKYELLOW    0x808000
#define LCD_COLOR_WHITE         0xFFFFFF
#define LCD_COLOR_LIGHTGRAY     0xD3D3D3
#define LCD_COLOR_GRAY          0x808080
#define LCD_COLOR_DARKGRAY      0x404040
#define LCD_COLOR_BLACK         0x000000
#define LCD_COLOR_BROWN         0xA52A2A
#define LCD_COLOR_ORANGE        0xFFA500
#define LCD_COLOR_BLACK         0x000000

/* Timing configuration from datahseet
  HSYNC=10 (9+1)
  HBP=20 (29-10+1)
  ActiveW=240 (269-20-10+1)
  HFP=10 (279-240-20-10+1)

  VSYNC=2 (1+1)
  VBP=2 (3-2+1)
  ActiveH=320 (323-2-2+1)
  VFP=4 (327-320-2-2+1)
*/
#define  ILI9341_HSYNC            ((uint32_t)9)   /* Horizontal synchronization */
#define  ILI9341_HBP              ((uint32_t)29)    /* Horizontal back porch      */
#define  ILI9341_HFP              ((uint32_t)2)    /* Horizontal front porch     */
#define  ILI9341_VSYNC            ((uint32_t)1)   /* Vertical synchronization   */
#define  ILI9341_VBP              ((uint32_t)3)    /* Vertical back porch        */
#define  ILI9341_VFP              ((uint32_t)2)    /* Vertical front porch       */
#define  LCD_PIXEL_WIDTH_X    ILI9341_LCD_PIXEL_WIDTH
#define  LCD_PIXEL_HEIGHT_Y   ILI9341_LCD_PIXEL_HEIGHT
#define  LCD_PIXELS		     ((uint32_t)LCD_PIXEL_WIDTH_X * (uint32_t)LCD_PIXEL_HEIGHT_Y)

void LTCD__Init(void);
void LTCD_Layer_Init(uint8_t LayerIndex);

void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);
void LCD_SetTextColor(uint32_t Color);
void LCD_SetFont(FONT_t *fonts);

// Draw Circle Filled
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint32_t color);

// Draw Vertical Line
void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint32_t color);
void LCD_Clear(uint32_t Color);

void LCD_Error_Handler(void);

// Demo using provided functions
void visualDemo(void);

void LCD_Error_Handler(void);

#if COMPILE_TOUCH_FUNCTIONS == 1
void InitializeLCDTouch(void);
STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct);
void LCD_Touch_Polling_Demo(void);
void DetermineTouchPosition(STMPE811_TouchData * touchStruct);
uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead);
void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData);

#endif // COMPILE_TOUCH_FUNCTIONS


/*        APPLICATION SPECIFIC FUNCTION DECLARATION - PUT YOUR NEWLY CREATED FUNCTIONS HERE       */


#endif /* INC_LCD_DRIVER_H_ */
