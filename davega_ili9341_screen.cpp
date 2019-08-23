/*
    Copyright 2019 Jan Pomikalek <jan.pomikalek@gmail.com>

    This file is part of the DAVEga firmware.

    DAVEga firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DAVEga firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DAVEga firmware.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "davega_ili9341_screen.h"

// pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

#define TFT_DC  9
#define TFT_CS 10


ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
ILI9341_t3* p_tft = nullptr;

#define TCLK 19
#define TCS 20
#define TDIN 21
#define DOUT 22
#define IRQ 23

URTouch touch(IRQ, DOUT, TDIN, TCS, TCLK);
URTouch* p_touch = nullptr;

void DavegaILI9341Screen::init(t_davega_screen_config *config) {
    DavegaScreen::init(config);
    if (!p_tft) {
        p_tft = &tft;
        p_tft->begin();
        p_tft->setRotation(config->orientation);
        p_tft->fillScreen(ILI9341_BLACK);
        Serial.println("initialized screen");

    }

    if (!p_touch) {
      p_touch = &touch;
      p_touch->InitTouch();
      p_touch->setPrecision(PREC_MEDIUM);
    }

    _tft = p_tft;
    _touch = p_touch;    
}
