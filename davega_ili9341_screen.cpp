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

// For the Adafruit shield, these are the default.
#define TFT_MISO 12
#define TFT_RST 9
#define TFT_DC  8
#define TFT_CS  10
#define TFT_MOSI 11
#define TFT_CLK 13

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
Adafruit_ILI9341* p_tft = nullptr;

#define TCLK 3
#define TCS 4
#define TDIN 5
#define DOUT 6
#define IRQ 7

URTouch touch = URTouch(IRQ, DOUT, TDIN, TCS, TCLK);
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
