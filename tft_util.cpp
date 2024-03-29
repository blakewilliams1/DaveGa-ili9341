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

#include "tft_util.h"

// TODO: PROGMEM
const bool FONT_DIGITS_3x5[10][5][3] = {
        {
                {1, 1, 1},
                {1, 0, 1},
                {1, 0, 1},
                {1, 0, 1},
                {1, 1, 1},
        },
        {
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
        },
        {
                {1, 1, 1},
                {0, 0, 1},
                {1, 1, 1},
                {1, 0, 0},
                {1, 1, 1},
        },
        {
                {1, 1, 1},
                {0, 0, 1},
                {0, 1, 1},
                {0, 0, 1},
                {1, 1, 1},
        },
        {
                {1, 0, 1},
                {1, 0, 1},
                {1, 1, 1},
                {0, 0, 1},
                {0, 0, 1},
        },
        {
                {1, 1, 1},
                {1, 0, 0},
                {1, 1, 1},
                {0, 0, 1},
                {1, 1, 1},
        },
        {
                {1, 1, 1},
                {1, 0, 0},
                {1, 1, 1},
                {1, 0, 1},
                {1, 1, 1},
        },
        {
                {1, 1, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
                {0, 0, 1},
        },
        {
                {1, 1, 1},
                {1, 0, 1},
                {1, 1, 1},
                {1, 0, 1},
                {1, 1, 1},
        },
        {
                {1, 1, 1},
                {1, 0, 1},
                {1, 1, 1},
                {0, 0, 1},
                {1, 1, 1},
        }
};

void tft_util_draw_digit(
        ILI9341_t3* tft, uint8_t digit, uint16_t x, uint16_t y,
        uint16_t fg_color, uint16_t bg_color, uint8_t magnify) {
    for (int xx = 0; xx < 3; xx++) {
        for (int yy = 0; yy < 5; yy++) {
            uint16_t color = FONT_DIGITS_3x5[digit][yy][xx] ? fg_color : bg_color;
            int x1 = x + xx * magnify;
            int y1 = y + yy * magnify;
            tft->fillRect(x1, y1, magnify, magnify, color);
        }
    }
}

void tft_util_draw_number(
        ILI9341_t3* tft, char *number, uint16_t x, uint16_t y,
        uint16_t fg_color, uint16_t bg_color, uint8_t spacing, uint8_t magnify) {
    int cursor_x = x;
    int number_len = strlen(number);
    for (int i=0; i < number_len; i++) {
        char ch = number[i];
        if (ch >= '0' and ch <= '9') {
            tft_util_draw_digit(tft, ch - '0', cursor_x, y, fg_color, bg_color, magnify);
            cursor_x += 3 * magnify + spacing;
        } else if (ch == '.') {
            tft->fillRect(cursor_x, y, magnify - 1, 5 * magnify, bg_color);
            tft->fillRect(cursor_x, y + 4 * magnify, magnify, 5 * magnify - 4 * magnify, fg_color);
            cursor_x += magnify + spacing;
        } else if (ch == '-') {
            tft->fillRect(cursor_x, y, 3 * magnify, 5 * magnify, bg_color);
            tft->fillRect(cursor_x, y + 2 * magnify, 3 * magnify, 3 * magnify - (2 * magnify), fg_color);
            cursor_x += 3 * magnify + spacing;
        } else if (ch == ' ') {
            tft->fillRect(cursor_x, y, 3 * magnify, 5 * magnify, bg_color);
            cursor_x += 3 * magnify + spacing;
        }
    }
}

uint16_t progress_to_color(float progress, ILI9341_t3* tft) {
    float brightness = 255.0 * (1.0 - progress);
    return  tft->color565(brightness,brightness,brightness);//tft->setColor(brightness, brightness, brightness);
}

void updateHighlighting(Button oldButton, Button newButton, ILI9341_t3* tft) {
  tft->drawRect(oldButton.x - 2, oldButton.y - 2, oldButton.width + 4, 2, ILI9341_BLACK);
  tft->drawRect(oldButton.x + oldButton.width, oldButton.y - 2, 2, oldButton.height + 4, ILI9341_BLACK);
  tft->drawRect(oldButton.x - 2, oldButton.y + oldButton.height, oldButton.width + 4, 2, ILI9341_BLACK);
  tft->drawRect(oldButton.x - 2, oldButton.y - 2, 2, oldButton.height + 4, ILI9341_BLACK);

  tft->drawRect(newButton.x - 2, newButton.y - 2, newButton.width + 4, 2, ILI9341_RED);
  tft->drawRect(newButton.x + newButton.width, newButton.y - 2, 2, newButton.height + 4, ILI9341_RED);
  tft->drawRect(newButton.x - 2, newButton.y + newButton.height, newButton.width + 4, 2, ILI9341_RED);
  tft->drawRect(newButton.x - 2, newButton.y - 2, 2, newButton.height + 4, ILI9341_RED);
}
