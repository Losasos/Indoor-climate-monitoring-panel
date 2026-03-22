#include "EPD_CODICO.h"
#include "EPD_Data.h"

void EPD_SendData(UBYTE Data)
{
    //Send data byte
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

void EPD_SendCommand(UBYTE Reg)
{
    // Read Busy flag - send command only when Busy is High
    while(DEV_Digital_Read(EPD_BUSY_PIN) == 0) { delayMicroseconds(1); }
    // Send command byte
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

void EPD_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    delayMicroseconds(10);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    delayMicroseconds(10);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    delayMicroseconds(10);
}

void EPD_SendImageData(char color) // BLACK, WHITE inverts BLACK() or RED
{
    UBYTE i;
    if (color == BLACK) 
    {
        EPD_SendCommand(0x10); 
        for (int i = 0; i < EPD_MATRIX_SIZE; i++) 
        {
            EPD_SendData(DISPLAY_BW_IMAGE[i]);
        }  
    }
    else if (color == WHITE) // inverts BLACK color data 
    {
        EPD_SendCommand(0x10); 
        for (int i = 0; i < EPD_MATRIX_SIZE; i++) 
        {
            EPD_SendData(255 - DISPLAY_BW_IMAGE[i]);
        }  
    }
    else if (color == RED) 
    {
        EPD_SendCommand(0x13);
        for (int i = 0; i < EPD_MATRIX_SIZE; i++) 
        {
            EPD_SendData(DISPLAY_RY_IMAGE[i]);
        }  
    }    
}

void EPD_Init(void)
{
    delay(10);
    EPD_Reset();
    EPD_SendCommand(0x04);		// Power ON
    Serial.print("Reset and init = OK\r\n");	
}

void EPD_SetDisplayColor(char color)  // BLACK, WHITE or RED
{
	  UWORD i;
    if (color == WHITE)
    {
        EPD_SendCommand(0x10);    //Load Image
        for(i=0; i<EPD_MATRIX_SIZE; i++)
        {
            EPD_SendData(0xFF); // => WHITE screen
        }
        EPD_SendCommand(0x13);    //Load Image
        for(i=0; i<EPD_MATRIX_SIZE; i++)
        {
            EPD_SendData(0x00); 
        }
    }
    else if (color == BLACK)
    {
	      EPD_SendCommand(0x10);   	//Load Image
	      for(i=0; i<EPD_MATRIX_SIZE; i++)
	      {
    	      EPD_SendData(0x00); // => BLACK screen
    	  }
        EPD_SendCommand(0x13);    //Load Image
        for(i=0; i<EPD_MATRIX_SIZE; i++)
        {
            EPD_SendData(0x00); 
        }
    }
    else if (color == RED)
    {
        EPD_SendCommand(0x10);    //Load Image
        for(i=0; i<EPD_MATRIX_SIZE; i++)
        {
            EPD_SendData(0xFF); // => WHITE screen
        }
        EPD_SendCommand(0x13);    //Load Image
        for(i=0; i<EPD_MATRIX_SIZE; i++)
        {
            EPD_SendData(0xFF); // => RED screen
        }
    }
}

void EPD_Refresh(void)
{
    EPD_SendCommand(0x12);
    Serial.print("Display refresh ...\r\n");
}

void EPD_Sleep(void)
{
    EPD_SendCommand(0x07);
    EPD_SendData(0xA5);
	  Serial.print("Deep sleep mode set ...\r\n");
}

void EPD_Pixel(short x, short y, short color)
{
    int n;
    short b;
    short c;
    if(color == RED)
    {
        n = x/8 + y*(EPD_WIDTH/8);
        b = 7 - x%8;
        c = 0x01;
        DISPLAY_RY_IMAGE[n] = DISPLAY_RY_IMAGE[n] & ~(1<<b); // clear bit
        DISPLAY_RY_IMAGE[n] = DISPLAY_RY_IMAGE[n] | (c<<b);  // set color bit
    }
    else
    {
        n = x/8 + y*(EPD_WIDTH/8);
        b = 7 - x%8;
        c = color & 0x01;
        DISPLAY_BW_IMAGE[n] = DISPLAY_BW_IMAGE[n] & ~(1<<b); // clear bit
        DISPLAY_BW_IMAGE[n] = DISPLAY_BW_IMAGE[n] | (c<<b);  // set color bit
        DISPLAY_RY_IMAGE[n] = DISPLAY_RY_IMAGE[n] & ~(1<<b); // clear bit @ RED
    }
}

void EPD_Line(short Xstart, short Ystart, short Xend, short Yend, short color, char skip)
{
    if (Xstart >= EPD_WIDTH) { Xstart = EPD_WIDTH - 1; }
    if (Xstart < 0) { Xstart = 0; }
    if (Xend >= EPD_WIDTH) { Xend = EPD_WIDTH - 1; }
    if (Xend < 0) { Xend = 0; }
    if (Ystart >= EPD_HEIGHT) { Ystart = EPD_HEIGHT - 1; }
    if (Ystart < 0) { Ystart = 0; }
    if (Yend >= EPD_HEIGHT) { Yend = EPD_HEIGHT - 1; }
    if (Yend < 0) { Yend = 0; }
    //
    short Xpoint = Xstart;
    short Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;
    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;
    //Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;
    for (;;) {
        Dotted_Len++;
        if (skip == 0) 
        { EPD_Pixel(Xpoint, Ypoint, color); }        
        else if (Dotted_Len%(skip + skip) < skip) 
        { EPD_Pixel(Xpoint, Ypoint, color); }        
        //
        if (2 * Esp >= dy) 
        {
            if (Xpoint == Xend) break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) 
        {
            if (Ypoint == Yend) break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void EPD_Rectangle(short Xstart, short Ystart, short Xend, short Yend, short color, char skip, char fill)
{
    if (Xstart >= EPD_WIDTH) { Xstart = EPD_WIDTH - 1; }
    if (Xstart < 0) { Xstart = 0; }
    if (Xend >= EPD_WIDTH) { Xend = EPD_WIDTH - 1; }
    if (Xend < 0) { Xend = 0; }
    if (Ystart >= EPD_HEIGHT) { Ystart = EPD_HEIGHT - 1; }
    if (Ystart < 0) { Ystart = 0; }
    if (Yend >= EPD_HEIGHT) { Yend = EPD_HEIGHT - 1; }
    if (Yend < 0) { Yend = 0; }
    short tmp;
    if (Xstart > Xend) 
    {
      tmp = Xstart;
      Xstart = Xend;
      Xend = tmp;
    }
    if (Ystart > Yend) 
    {
      tmp = Ystart;
      Ystart = Yend;
      Yend = tmp;
    }
    //
    if (fill) 
    {
        short Xpoint, Ypoint;
        for(Ypoint = Ystart; Ypoint < Yend; Ypoint++) 
        {
          for(Xpoint = Xstart; Xpoint < Xend; Xpoint++)
          {
            if (skip)
            {
              if ((Xpoint%2 == 0) && (Ypoint%2 == 0)) { EPD_Pixel(Xpoint, Ypoint, color); }
              if ((Xpoint%2 == 1) && (Ypoint%2 == 1)) { EPD_Pixel(Xpoint, Ypoint, color); }
            }
            else 
            {
              EPD_Pixel(Xstart, Ypoint, color);
            }
          }
        }
    } 
    else 
    {
        EPD_Line(Xstart, Ystart, Xend, Ystart, color, skip);
        EPD_Line(Xstart, Ystart, Xstart, Yend, color, skip);
        EPD_Line(Xend, Yend, Xend, Ystart, color, skip);
        EPD_Line(Xend, Yend, Xstart, Yend, color, skip);
    }
}

void EPD_Circle(short X_Center, short Y_Center, short Radius, short color, char fill)
{
    if (X_Center >= EPD_WIDTH) { X_Center = EPD_WIDTH - 1; }
    if (Y_Center >= EPD_HEIGHT) { Y_Center = EPD_HEIGHT - 1; }
    //Draw a circle from(0, R) as a starting point
    short XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;
    //Cumulative error,judge the next point of the logo
    short Esp = 3 - (Radius << 1);
    short sCountY;
    if (fill) 
    {
        while (XCurrent <= YCurrent) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++) 
            {
                EPD_Pixel(X_Center + XCurrent, Y_Center + sCountY, color);  //1
                EPD_Pixel(X_Center - XCurrent, Y_Center + sCountY, color);  //2
                EPD_Pixel(X_Center - sCountY, Y_Center + XCurrent, color);  //3
                EPD_Pixel(X_Center - sCountY, Y_Center - XCurrent, color);  //4
                EPD_Pixel(X_Center - XCurrent, Y_Center - sCountY, color);  //5
                EPD_Pixel(X_Center + XCurrent, Y_Center - sCountY, color);  //6
                EPD_Pixel(X_Center + sCountY, Y_Center - XCurrent, color);  //7
                EPD_Pixel(X_Center + sCountY, Y_Center + XCurrent, color);  //0
            }
            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } 
    else 
    { //Draw a hollow circle
        while (XCurrent <= YCurrent) 
        {
            EPD_Pixel(X_Center + XCurrent, Y_Center + YCurrent, color); //1
            EPD_Pixel(X_Center - XCurrent, Y_Center + YCurrent, color); //2
            EPD_Pixel(X_Center - YCurrent, Y_Center + XCurrent, color); //3
            EPD_Pixel(X_Center - YCurrent, Y_Center - XCurrent, color); //4
            EPD_Pixel(X_Center - XCurrent, Y_Center - YCurrent, color); //5
            EPD_Pixel(X_Center + XCurrent, Y_Center - YCurrent, color); //6
            EPD_Pixel(X_Center + YCurrent, Y_Center - XCurrent, color); //7
            EPD_Pixel(X_Center + YCurrent, Y_Center + XCurrent, color); //0
            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

void EPD_AddFontImage(short Xstart, short Ystart, short color, short index) // index in the FONT_IMG array
{
    int n = Xstart/8 + Ystart*(30); // 30 = EPD_WIDTH/8
    for (int i=0; i<EPD_SYM_SIZE; i++)
    {   
        if(color == RED)
        {
            DISPLAY_BW_IMAGE[n] = 0xFF;
            DISPLAY_RY_IMAGE[n] = FONT_IMG[index][i];
        }
        else
        {
            DISPLAY_BW_IMAGE[n] = FONT_IMG[index][i];
            DISPLAY_RY_IMAGE[n] = 0x00;
        }
        n++;
        if ((i%3) == 2) { n = n + 27; }
    }
}

void EPD_Test(short Xstart, short Ystart, short index) // Test graphic function
{
    int n = Xstart/8 + Ystart*(30); // 30 = EPD_WIDTH/8
    for (int i=0; i<EPD_SYM_SIZE; i++)
    {   
        DISPLAY_RY_IMAGE[n] = 0xF0;
        DISPLAY_BW_IMAGE[n] = 0x00;
        n++;
        if ((i%3) == 2) { n = n + 27; }
    }
}
