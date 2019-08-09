/*
    Copyright 2018 Jan Pomikalek <jan.pomikalek@gmail.com>

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

#ifndef TFT_UTIL_H
#define TFT_UTIL_H

#include <Adafruit_ILI9341.h>

void tft_util_draw_digit(
        Adafruit_ILI9341* tft, uint8_t digit, uint8_t x, uint8_t y,
        uint16_t fg_color, uint16_t bg_color, uint8_t magnify = 1);

void tft_util_draw_number(
        Adafruit_ILI9341* tft, char *number, uint8_t x, uint8_t y,
        uint16_t fg_color, uint16_t bg_color, uint8_t spacing, uint8_t magnify = 1);

uint16_t progress_to_color(float progress, Adafruit_ILI9341* tft);

#endif //TFT_UTIL_H
