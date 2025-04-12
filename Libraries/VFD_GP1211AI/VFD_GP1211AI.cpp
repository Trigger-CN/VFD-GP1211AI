/*
 * MIT License
 * Copyright (c) 2025 _VIFEXTech, Trigger-CN
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
#include "VFD_GP1211AI.h"
#include "SPI.h"

#define DISPLAY_GP1211AI_WIDTH 128
#define DISPLAY_GP1211AI_HEIGHT 64

#define VFD_CLKG_PIN_STROBE()          \
    {                                  \
        digitalWrite(_clkg_pin, LOW);  \
        digitalWrite(_clkg_pin, HIGH); \
    }                                  \
    while (0)

#define VFD_LAT_PIN_STROBE()          \
    {                                 \
        digitalWrite(_lat_pin, LOW);  \
        digitalWrite(_lat_pin, HIGH); \
    }                                 \
    while (0)

VFD_GP1211AI::VFD_GP1211AI(
    uint8_t bk_pin,
    uint8_t lat_pin,
    uint8_t sig_pin,
    uint8_t clkg_pin,
    uint8_t hv_en_pin,
    uint8_t fl_en_pin,
    SPIClass* spi)
    : Adafruit_GFX(DISPLAY_GP1211AI_WIDTH, DISPLAY_GP1211AI_HEIGHT)
    , _bk_pin(bk_pin)
    , _lat_pin(lat_pin)
    , _sig_pin(sig_pin)
    , _clkg_pin(clkg_pin)
    , _hv_en_pin(hv_en_pin)
    , _fl_en_pin(fl_en_pin)
    , _spi(spi)
    , _brightness(0)
    , _sendPtr(0)
    , _gridScanCnt(0)
{
    memset(_framebuffer, 0, sizeof(_framebuffer));
    memset(_sendBuffer, 0, sizeof(_sendBuffer));
}

void VFD_GP1211AI::begin()
{
    pinMode(_lat_pin, OUTPUT);
    pinMode(_sig_pin, OUTPUT);
    pinMode(_clkg_pin, OUTPUT);
    pinMode(_hv_en_pin, OUTPUT);
    pinMode(_fl_en_pin, OUTPUT);

    _spi->begin();
    _spi->setClock(12000000);
    _spi->setBitOrder(LSBFIRST);

    digitalWrite(_hv_en_pin, LOW);
    digitalWrite(_fl_en_pin, HIGH);

    setBrightness(128);

    digitalWrite(_fl_en_pin, LOW);
    delay(100);
    digitalWrite(_hv_en_pin, HIGH);
}

void VFD_GP1211AI::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
        return;

    uint8_t* buffer = (uint8_t*)_framebuffer;

    switch (color) {
    case WHITE:
        buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
        break;
    case BLACK:
        buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
        break;
    case INVERSE:
        buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
        break;
    }
}

void VFD_GP1211AI::drawFastVLine(int16_t x, int16_t __y, int16_t __h,
    uint16_t color)
{
    // do nothing if we're off the left or right side of the screen
    if (x < 0 || x >= WIDTH) {
        return;
    }

    // make sure we don't try to draw below 0
    if (__y < 0) {
        // __y is negative, this will subtract enough from __h to account for __y
        // being 0
        __h += __y;
        __y = 0;
    }

    // make sure we don't go past the height of the display
    if ((__y + __h) > HEIGHT) {
        __h = (HEIGHT - __y);
    }

    // if our height is now negative, punt
    if (__h <= 0) {
        return;
    }

    // this display doesn't need ints for coordinates, use local byte registers
    // for faster juggling
    register uint8_t y = __y;
    register uint8_t h = __h;

    // set up the pointer for fast movement through the buffer
    register uint8_t* pBuf = (uint8_t*)_framebuffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * WIDTH);
    // and offset x columns in
    pBuf += x;

    // do the first partial byte, if necessary - this requires some masking
    register uint8_t mod = (y & 7);
    if (mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;

        // note - lookup table results in a nearly 10% performance improvement in
        // fill* functions register uint8_t mask = ~(0xFF >> (mod));
        static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0,
            0xF0, 0xF8, 0xFC, 0xFE };
        register uint8_t mask = premask[mod];

        // adjust the mask if we're not going to reach the end of this byte
        if (h < mod) {
            mask &= (0XFF >> (mod - h));
        }

        switch (color) {
        case WHITE:
            *pBuf |= mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^= mask;
            break;
        }

        // fast exit if we're done here!
        if (h < mod) {
            return;
        }

        h -= mod;

        pBuf += WIDTH;
    }

    // write solid bytes while we can - effectively doing 8 rows at a time
    if (h >= 8) {
        if (color == INVERSE) { // separate copy of the code so we don't impact
            // performance of the black/white write version with
            // an extra comparison per loop
            do {
                *pBuf = ~(*pBuf);

                // adjust the buffer forward 8 rows worth of data
                pBuf += WIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but
                // this should still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        } else {
            // store a local value to work with
            register uint8_t val = (color == WHITE) ? 255 : 0;

            do {
                // write our value in
                *pBuf = val;

                // adjust the buffer forward 8 rows worth of data
                pBuf += WIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but
                // this should still help a fair bit for now)
                h -= 8;
            } while (h >= 8);
        }
    }

    // now do the final partial byte, if necessary
    if (h) {
        mod = h & 7;
        // this time we want to mask the low bits of the byte, vs the high bits we
        // did above register uint8_t mask = (1 << mod) - 1; note - lookup table
        // results in a nearly 10% performance improvement in fill* functions
        static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07,
            0x0F, 0x1F, 0x3F, 0x7F };
        register uint8_t mask = postmask[mod];
        switch (color) {
        case WHITE:
            *pBuf |= mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^= mask;
            break;
        }
    }
}

void VFD_GP1211AI::drawFastHLine(int16_t x, int16_t y, int16_t w,
    uint16_t color)
{
    // Do bounds/limit checks
    if (y < 0 || y >= HEIGHT) {
        return;
    }

    // make sure we don't try to draw below 0
    if (x < 0) {
        w += x;
        x = 0;
    }

    // make sure we don't go off the edge of the display
    if ((x + w) > WIDTH) {
        w = (WIDTH - x);
    }

    // if our width is now negative, punt
    if (w <= 0) {
        return;
    }

    // set up the pointer for  movement through the buffer
    register uint8_t* pBuf = (uint8_t*)_framebuffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * WIDTH);
    // and offset x columns in
    pBuf += x;

    register uint8_t mask = 1 << (y & 7);

    switch (color) {
    case WHITE:
        while (w--) {
            *pBuf++ |= mask;
        };
        break;
    case BLACK:
        mask = ~mask;
        while (w--) {
            *pBuf++ &= mask;
        };
        break;
    case INVERSE:
        while (w--) {
            *pBuf++ ^= mask;
        };
        break;
    }
}

void VFD_GP1211AI::fillScreen(uint16_t color)
{
    color = color ? 0xFF : 0x00;
    memset(_framebuffer, color, sizeof(_framebuffer));
}

void VFD_GP1211AI::display(void)
{
    uint8_t i = 0;
    uint8_t grid;
    uint8_t grid_tmp = 0;
    uint8_t ram_tmp = 0;
    uint8_t ram_base_addr = 0;
    uint8_t* buf_ptr;

    buf_ptr = _sendBuffer;

    for (grid = 0; grid < 43; grid++) {
        grid_tmp = 42 - grid;

        ram_base_addr = (grid_tmp >> 1) * 6;
        for (i = 0; i < 8; i++) {
            if (grid_tmp & 0x01) //双数列
            {
                ram_tmp = ((_framebuffer[i][ram_base_addr + 5] << 1) & 0x02);
                ram_tmp |= ((_framebuffer[i][ram_base_addr + 4] << 3) & 0x08);
                ram_tmp |= ((_framebuffer[i][ram_base_addr + 3] << 5) & 0x20);
                ram_tmp |= ((_framebuffer[i][ram_base_addr + 5] << 6) & 0x80);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = _framebuffer[i][ram_base_addr + 4] & 0x02;
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 3]) << 2) & 0x08);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 5]) << 3) & 0x20);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 4]) << 5) & 0x80);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (((_framebuffer[i][ram_base_addr + 3]) >> 1) & 0x02);
                ram_tmp |= _framebuffer[i][ram_base_addr + 5] & 0x08;
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 4]) << 2) & 0x20);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 3]) << 4) & 0x80);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (((_framebuffer[i][ram_base_addr + 5]) >> 3) & 0x02);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 4]) >> 1) & 0x08);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 3]) << 1) & 0x20);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 5]) << 2) & 0x80);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (((_framebuffer[i][ram_base_addr + 4]) >> 4) & 0x02);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 3]) >> 2) & 0x08);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 5]) >> 1) & 0x20);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 4]) << 1) & 0x80);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (((_framebuffer[i][ram_base_addr + 3]) >> 5) & 0x02);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 5]) >> 4) & 0x08);
                ram_tmp |= (((_framebuffer[i][ram_base_addr + 4]) >> 2) & 0x20);
                ram_tmp |= _framebuffer[i][ram_base_addr + 3] & 0x80;
                *buf_ptr = ram_tmp;
                ++buf_ptr;
            } else //单数列
            {
                ram_tmp = _framebuffer[i][ram_base_addr + 0] & 0x01;
                ram_tmp |= (_framebuffer[i][ram_base_addr + 1] << 2 & 0x04);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 2] << 4 & 0x10);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] << 5 & 0x40);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (_framebuffer[i][ram_base_addr + 1] >> 1 & 0x01);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 2] << 1 & 0x04);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] << 2 & 0x10);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 1] << 4 & 0x40);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (_framebuffer[i][ram_base_addr + 2] >> 2 & 0x01);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] >> 1 & 0x04);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 1] << 1 & 0x10);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 2] << 3 & 0x40);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (_framebuffer[i][ram_base_addr + 0] >> 4 & 0x01);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 1] >> 2 & 0x04);
                ram_tmp |= _framebuffer[i][ram_base_addr + 2] & 0x10;
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] << 1 & 0x40);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (_framebuffer[i][ram_base_addr + 1] >> 5 & 0x01);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 2] >> 3 & 0x04);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] >> 2 & 0x10);
                ram_tmp |= _framebuffer[i][ram_base_addr + 1] & 0x40;
                *buf_ptr = ram_tmp;
                ++buf_ptr;
                ram_tmp = (_framebuffer[i][ram_base_addr + 2] >> 6 & 0x01);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 0] >> 5 & 0x04);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 1] >> 3 & 0x10);
                ram_tmp |= (_framebuffer[i][ram_base_addr + 2] >> 1 & 0x40);
                *buf_ptr = ram_tmp;
                ++buf_ptr;
            }
        }
    }
}

void VFD_GP1211AI::timerHandler(void)
{
    uint8_t cycle_cnt = 48;

    if (_gridScanCnt == 0) {
        _sendPtr = _sendBuffer;
        _gridScanCnt = 43;

        digitalWrite(_sig_pin, HIGH);
        VFD_CLKG_PIN_STROBE();
        VFD_CLKG_PIN_STROBE();
        digitalWrite(_sig_pin, LOW);
        VFD_CLKG_PIN_STROBE();
        VFD_CLKG_PIN_STROBE();
        VFD_CLKG_PIN_STROBE();
    }

    //    while ((cycle_cnt--) > 0) {
    //       _spi->write(*ptr++);
    //    }

    _spi->write(_sendPtr, cycle_cnt);
    _sendPtr += cycle_cnt;
    VFD_CLKG_PIN_STROBE();
    analogWrite(_bk_pin, 0);

    VFD_LAT_PIN_STROBE();
    analogWrite(_bk_pin, 255 - _brightness);
    --_gridScanCnt;
}
