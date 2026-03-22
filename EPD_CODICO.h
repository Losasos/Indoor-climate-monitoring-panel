#ifndef __EPD_CODICO_
#define __EPD_CODICO_

#include "DEV_Config.h"

// Display parameters :: DEPG0370xxU
#define EPD_WIDTH       240
#define EPD_HEIGHT      416
#define EPD_MATRIX_SIZE 12480

// Colors
#define BLACK  0x00
#define WHITE  0xFF
#define RED    0x01  // use RED if any color

#define SOLID  0  // Solid and dash lines
#define DASH_1 1
#define DASH_2 2
#define DASH_4 4
#define DASH_8 8

// Basic communication functions
void EPD_SendCommand(uint8_t Reg);
void EPD_SendData(uint8_t Data);
void EPD_Reset(void);

// Main EPD functions
void EPD_Init(void);
void EPD_SetDisplayColor(char color);
void EPD_SendImageData(char color);
void EPD_Refresh(void);
void EPD_Sleep(void);

// Graphic primitives functions
void EPD_Pixel(short x, short y, short color);
void EPD_Line(short Xstart, short Ystart, short Xend, short Yend, short color, char skip);
void EPD_Rectangle(short Xstart, short Ystart, short Xend, short Yend, short color, char skip, char fill);
void EPD_Circle(short X_Center, short Y_Center, short Radius, short color, char fill);

// Font specific functions
void EPD_AddFontImage(short Xstart, short Ystart, short color, short index);

#endif
