#ifndef __LCD_H
#define __LCD_H

#include <stdio.h>
#include <stdlib.h>

#include "Font_05x07.h"
#include "Font_07x09.h"
#include "Font_10x14.h"
#include "Font_14x18.h"
#include "Global.h"
#include "Icon.h"
#include "NT35510.h"
#include "main.h"
#include "string.h"

class Lcd {
   private:
    uint16_t width;
    uint16_t height;
    RGB16Color foreColor;
    RGB16Color backColor;
    LcdRotation rotation;
    LcdFont font;
    LcdAlign fontAlignment;
    const uint16_t* fontData;
    uint16_t fontSpacing;
    uint16_t fontWidth;
    uint16_t fontHeight;
    void writeCommand(uint16_t command) {
        LCD_RS_LOW;
        LCD_DATA_GPIO_Port->ODR = command;
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
    void writeData(uint16_t data) {
        LCD_RS_HIGH;
        LCD_DATA_GPIO_Port->ODR = data;
        LCD_WR_LOW;
        LCD_WR_HIGH;
    }
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void clearAddressWindow();
    void fastFill(RGB16Color color, uint32_t pix);
    uint8_t getLetter(char letter);
    RGB16Color alphaColor(RGB16Color color_, float alpha);
    RGB16Color alphaColor(struct RGB24RawColor color_, float alpha);
    RGB16Color convertColor(struct RGB24RawColor color_);

   public:
    Lcd(LcdRotation rotation_, RGB16Color foreColor_, RGB16Color backColor_, LcdFont font_);
    ~Lcd();
    uint16_t getWidth() const { return width; }
    uint16_t getHeight() const { return height; }
    void setForeColor(RGB16Color color_) { foreColor = color_; }
    RGB16Color getForeColor() const { return foreColor; }
    void setBackColor(RGB16Color color_) { backColor = color_; }
    RGB16Color getBackColor() const { return backColor; }
    void setRotation(LcdRotation rotation_);
    LcdRotation getRotation() const { return rotation; }
    void setFont(LcdFont font_);
    LcdFont getFont() const { return font; }
    void setSpacing(uint16_t spacing_) { fontSpacing = spacing_; }
    uint16_t getSpacing() const { return fontSpacing; }
    void setAlignment(LcdAlign align_) { fontAlignment = align_; }
    LcdAlign getAlignment() const { return fontAlignment; }
    void initialize();
    void clearScreen();
    void fillScreen();
    void fillScreen(RGB16Color color);
    void drawPixel(uint16_t x, uint16_t y);
    void drawHLine(uint16_t x, uint16_t y, uint16_t l);
    void drawVLine(uint16_t x, uint16_t y, uint16_t l);
    void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void drawRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h);
    void fillRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h);
    void drawCircle(uint16_t x, uint16_t y, uint16_t r);
    void drawCircleHelper(uint16_t x, uint16_t y, uint16_t r, uint8_t cornername);
    void drawQuarterCircle(uint16_t x, uint16_t y, uint16_t r, uint8_t quarter);
    void drawHalfCircle(uint16_t x, uint16_t y, uint16_t r, uint8_t half);
    void fillCircle(uint16_t x, uint16_t y, uint16_t r);
    void fillCircleHelper(uint16_t x, uint16_t y, uint16_t r, uint8_t cornername, uint16_t delta);
    void drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
    void fillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);
    void clearHLine(uint16_t x, uint16_t y, uint16_t l);
    void clearVLine(uint16_t x, uint16_t y, uint16_t l);
    void clearRect(uint16_t x, uint16_t y, uint32_t w, uint32_t h);
    void drawRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, float alpha, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void drawRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, float alpha, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void fadeRGB16Image(const RGB16Color* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool mode, uint8_t step, uint16_t delay);
    void fadeRGB24Image(const RGB24RawColor* indexPtr, const uint8_t* pixelPtr, uint8_t indexSize, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool mode, uint8_t step, uint16_t delay);
    void drawSdFirmwareAlert(bool type);
    void clearSdFirmwareAlert();
    void drawText(const char text[], uint16_t textLength, uint16_t x, uint16_t y);
    void drawNumber(int32_t num, uint8_t numSize, uint16_t x, uint16_t y);
    void invertDisplay(bool invert);
    void displayOn();
    void displayOff();
};

#endif
