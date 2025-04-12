/*
 * MIT License
 * Copyright (c) 2025 _VIFEXTech, TriggerCN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __VFD_GP1211AI_H
#define __VFD_GP1211AI_H

#include "Adafruit_GFX_Library/Adafruit_GFX.h"

#define BLACK 0
#define WHITE 1
#define INVERSE 2

class SPIClass;

class VFD_GP1211AI : public Adafruit_GFX {
public:
    VFD_GP1211AI(uint8_t bk_pin, uint8_t lat_pin, uint8_t sig_pin,
        uint8_t clkg_pin, uint8_t hv_en_pin, uint8_t fl_en_pin,
        SPIClass* spi);

    void begin();

    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void fillScreen(uint16_t color);
    void updateBuffer(void);
    void timerHandler(void);
    void setBrightness(uint16_t brightness)
    {
        _brightness = brightness;
        analogWrite(_bk_pin, 255 - _brightness);
    }
    uint16_t getBrightness(void) { return _brightness; }

private:
    uint8_t _bk_pin;
    uint8_t _lat_pin;
    uint8_t _sig_pin;
    uint8_t _clkg_pin;
    uint8_t _sia_pin;
    uint8_t _hv_en_pin;
    uint8_t _fl_en_pin;
    SPIClass* _spi;

    uint8_t _framebuffer[8][128];
    uint8_t _sendBuffer[2064];
    uint16_t _brightness;

    uint8_t* _sendPtr;
    uint8_t _gridScanCnt;
};

#endif /* __VFD_GP1211AI_H */
