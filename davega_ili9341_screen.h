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

#ifndef DAVEGA_ILI9341_SCREEN_H
#define DAVEGA_ILI9341_SCREEN_H

#include <ILI9341_t3.h> // Hardware-specific library
#include <SD.h>
#include <SPI.h>
#include <Time.h>
#include "davega_screen.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} Point;

class DavegaILI9341Screen: public DavegaScreen {
public:
    void init(t_davega_screen_config* config) override;

protected:

    ILI9341_t3* _tft;
};

#endif //DAVEGA_ILI9341_H
