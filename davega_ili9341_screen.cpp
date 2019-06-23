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

#define TFT_RST 12
#define TFT_RS  9
#define TFT_CS  10  // SS
#define TFT_SDI 11  // MOSI
#define TFT_CLK 13  // SCK
#define TFT_LED 0

// TODO: Fix constructor args if needed.
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_SDI);
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, 200);
Adafruit_ILI9341* p_tft = nullptr;

void DavegaILI9341Screen::init(t_davega_screen_config *config) {
    DavegaScreen::init(config);
    if (!p_tft) {
        p_tft = &tft;
        p_tft->begin();
        p_tft->setRotation(config->orientation);
        //p_tft->setBackgroundColor(ILI9341_BLACK);
        p_tft->fillRect(0,0, 220, 176, ILI9341_BLACK);
    }
    _tft = p_tft;
}
