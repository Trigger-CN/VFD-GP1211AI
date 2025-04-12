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
}

void loop()
{
    display.fillScreen(BLACK);
    display.drawCircle(50, 50, 20, WHITE);
    display.setCursor(0, 0);
    display.setTextColor(WHITE, BLACK);
    display.printf("Tick: %d", millis());
    display.drawRect(10, 10, 30, 40, WHITE);
    display.updateBuffer();
    delay(100);
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
