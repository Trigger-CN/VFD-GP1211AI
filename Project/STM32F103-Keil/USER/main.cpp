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
#include "Arduino.h"
#include "SPI.h"
#include "VFD_GP1211AI/VFD_GP1211AI.h"

#define VFD_BK_PIN PA0 // PWM6
#define VFD_LAT_PIN PA1
#define VFD_SIG_PIN PA2
#define VFD_CLKG_PIN PA3
#define VFD_SIA_PIN PA7 // MOSI
#define VFD_CLKA_PIN PA5 // SCLK
#define VFD_HV_EN_PIN PA4
#define VFD_FL_EN_PIN PA8

#define VFD_PFS 120
#define VFD_GRID_CNT 44
#define VFD_TIMER_PERIOD (1000000 / VFD_PFS / VFD_GRID_CNT)

static VFD_GP1211AI display(VFD_BK_PIN,
    VFD_LAT_PIN,
    VFD_SIG_PIN,
    VFD_CLKG_PIN,
    VFD_HV_EN_PIN,
    VFD_FL_EN_PIN,
    &SPI);

#define NUMFLAKES 10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
static const unsigned char PROGMEM logo_bmp[] = { 0b00000000, 0b11000000,
    0b00000001, 0b11000000,
    0b00000001, 0b11000000,
    0b00000011, 0b11100000,
    0b11110011, 0b11100000,
    0b11111110, 0b11111000,
    0b01111110, 0b11111111,
    0b00110011, 0b10011111,
    0b00011111, 0b11111100,
    0b00001101, 0b01110000,
    0b00011011, 0b10100000,
    0b00111111, 0b11100000,
    0b00111111, 0b11110000,
    0b01111100, 0b11110000,
    0b01110000, 0b01110000,
    0b00000000, 0b00110000 };

void testdrawline()
{
    int16_t i;

    display.clearDisplay(); // Clear display buffer

    for (i = 0; i < display.width(); i += 4) {
        display.drawLine(0, 0, i, display.height() - 1, WHITE);
        display.display(); // Update screen with each newly-drawn line
        delay(1);
    }
    for (i = 0; i < display.height(); i += 4) {
        display.drawLine(0, 0, display.width() - 1, i, WHITE);
        display.display();
        delay(1);
    }
    delay(250);

    display.clearDisplay();

    for (i = 0; i < display.width(); i += 4) {
        display.drawLine(0, display.height() - 1, i, 0, WHITE);
        display.display();
        delay(1);
    }
    for (i = display.height() - 1; i >= 0; i -= 4) {
        display.drawLine(0, display.height() - 1, display.width() - 1, i, WHITE);
        display.display();
        delay(1);
    }
    delay(250);

    display.clearDisplay();

    for (i = display.width() - 1; i >= 0; i -= 4) {
        display.drawLine(display.width() - 1, display.height() - 1, i, 0, WHITE);
        display.display();
        delay(1);
    }
    for (i = display.height() - 1; i >= 0; i -= 4) {
        display.drawLine(display.width() - 1, display.height() - 1, 0, i, WHITE);
        display.display();
        delay(1);
    }
    delay(250);

    display.clearDisplay();

    for (i = 0; i < display.height(); i += 4) {
        display.drawLine(display.width() - 1, 0, 0, i, WHITE);
        display.display();
        delay(1);
    }
    for (i = 0; i < display.width(); i += 4) {
        display.drawLine(display.width() - 1, 0, i, display.height() - 1, WHITE);
        display.display();
        delay(1);
    }

    delay(2000); // Pause for 2 seconds
}

void testdrawrect(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < display.height() / 2; i += 2) {
        display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, WHITE);
        display.display(); // Update screen with each newly-drawn rectangle
        delay(1);
    }

    delay(2000);
}

void testfillrect(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < display.height() / 2; i += 3) {
        // The INVERSE color is used so rectangles alternate white/black
        display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, INVERSE);
        display.display(); // Update screen with each newly-drawn rectangle
        delay(1);
    }

    delay(2000);
}

void testdrawcircle(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 2) {
        display.drawCircle(display.width() / 2, display.height() / 2, i, WHITE);
        display.display();
        delay(1);
    }

    delay(2000);
}

void testfillcircle(void)
{
    display.clearDisplay();

    for (int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 3) {
        // The INVERSE color is used so circles alternate white/black
        display.fillCircle(display.width() / 2, display.height() / 2, i, INVERSE);
        display.display(); // Update screen with each newly-drawn circle
        delay(1);
    }

    delay(2000);
}

void testdrawroundrect(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
        display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i,
            display.height() / 4, WHITE);
        display.display();
        delay(1);
    }

    delay(2000);
}

void testfillroundrect(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < display.height() / 2 - 2; i += 2) {
        // The INVERSE color is used so round-rects alternate white/black
        display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i,
            display.height() / 4, INVERSE);
        display.display();
        delay(1);
    }

    delay(2000);
}

void testdrawtriangle(void)
{
    display.clearDisplay();

    for (int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 5) {
        display.drawTriangle(
            display.width() / 2, display.height() / 2 - i,
            display.width() / 2 - i, display.height() / 2 + i,
            display.width() / 2 + i, display.height() / 2 + i, WHITE);
        display.display();
        delay(1);
    }

    delay(2000);
}

void testfilltriangle(void)
{
    display.clearDisplay();

    for (int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 5) {
        // The INVERSE color is used so triangles alternate white/black
        display.fillTriangle(
            display.width() / 2, display.height() / 2 - i,
            display.width() / 2 - i, display.height() / 2 + i,
            display.width() / 2 + i, display.height() / 2 + i, INVERSE);
        display.display();
        delay(1);
    }

    delay(2000);
}

void testdrawchar(void)
{
    display.clearDisplay();

    display.setTextSize(1); // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0); // Start at top-left corner
    display.cp437(true); // Use full 256 char 'Code Page 437' font

    // Not all the characters will fit on the display. This is normal.
    // Library will draw what it can and the rest will be clipped.
    for (int16_t i = 0; i < 256; i++) {
        if (i == '\n')
            display.write(' ');
        else
            display.write(i);
    }

    display.display();
    delay(2000);
}

void testdrawstyles(void)
{
    display.clearDisplay();

    display.setTextSize(1); // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0); // Start at top-left corner
    display.println(F("Hello, world!"));

    display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
    display.println(3.141592);

    display.setTextSize(2); // Draw 2X-scale text
    display.setTextColor(WHITE);
    display.print(F("0x"));
    display.println(0xDEADBEEF, HEX);

    display.display();
    delay(2000);
}

void testdrawbitmap(void)
{
    display.clearDisplay();

    display.drawBitmap(
        (display.width() - LOGO_WIDTH) / 2,
        (display.height() - LOGO_HEIGHT) / 2,
        logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
    display.display();
    delay(1000);
}

#define XPOS 0 // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2

void testanimate(const uint8_t* bitmap, uint8_t w, uint8_t h)
{
    int8_t f, icons[NUMFLAKES][3];

    // Initialize 'snowflake' positions
    for (f = 0; f < NUMFLAKES; f++) {
        icons[f][XPOS] = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS] = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
        Serial.print(F("x: "));
        Serial.print(icons[f][XPOS], DEC);
        Serial.print(F(" y: "));
        Serial.print(icons[f][YPOS], DEC);
        Serial.print(F(" dy: "));
        Serial.println(icons[f][DELTAY], DEC);
    }

    for (;;) { // Loop forever...
        display.clearDisplay(); // Clear the display buffer

        // Draw each snowflake:
        for (f = 0; f < NUMFLAKES; f++) {
            display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, WHITE);
        }

        display.display(); // Show the display buffer on the screen
        delay(200); // Pause for 1/10 second

        // Then update coordinates of each flake...
        for (f = 0; f < NUMFLAKES; f++) {
            icons[f][YPOS] += icons[f][DELTAY];
            // If snowflake is off the bottom of the screen...
            if (icons[f][YPOS] >= display.height()) {
                // Reinitialize to a random position, just off the top
                icons[f][XPOS] = random(1 - LOGO_WIDTH, display.width());
                icons[f][YPOS] = -LOGO_HEIGHT;
                icons[f][DELTAY] = random(1, 6);
            }
        }
    }
}

static void dispTimerHandler()
{
    display.timerHandler();
}

void setup()
{
    Serial.begin(115200);

    display.begin();

    Timer_SetInterrupt(TIM1, VFD_TIMER_PERIOD, dispTimerHandler);
    TIM_Cmd(TIM1, ENABLE);
    PWM_Init(VFD_BK_PIN, 255, 80 * 1000);

    display.setBrightness(255);

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel in white
    display.drawPixel(10, 10, WHITE);

    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();
    delay(2000);
    // display.display() is NOT necessary after every single drawing command,
    // unless that's what you want...rather, you can batch up a bunch of
    // drawing operations and then update the screen all at once by calling
    // display.display(). These examples demonstrate both approaches...

    testdrawline(); // Draw many lines

    testdrawrect(); // Draw rectangles (outlines)

    testfillrect(); // Draw rectangles (filled)

    testdrawcircle(); // Draw circles (outlines)

    testfillcircle(); // Draw circles (filled)

    testdrawroundrect(); // Draw rounded rectangles (outlines)

    testfillroundrect(); // Draw rounded rectangles (filled)

    testdrawtriangle(); // Draw triangles (outlines)

    testfilltriangle(); // Draw triangles (filled)

    testdrawchar(); // Draw characters of the default font

    testdrawstyles(); // Draw 'stylized' characters

    testdrawbitmap(); // Draw a small bitmap image

    // Invert and restore display, pausing in-between
    display.invertDisplay(true);
    delay(1000);
    display.invertDisplay(false);
    delay(1000);

    testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps
}

void loop()
{
}

/**
 * @brief  Main Function
 * @param  None
 * @retval None
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    GPIO_JTAG_Disable();
    //    SysClock_Init(F_CPU_48MHz);
    Delay_Init();
    ADCx_Init(ADC1);
    setup();
    for (;;)
        loop();
}
