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

#include "davega_default_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

#define BATTERY_INDICATOR_CELL_WIDTH 14
#define BATTERY_INDICATOR_CELL_HEIGHT 15

#define SPEED_INDICATOR_CELL_WIDTH 10
#define SPEED_INDICATOR_CELL_HEIGHT 11

#define LEN(X) (sizeof(X) / sizeof(X[0]))

typedef struct {
    uint8_t x;
    uint8_t y;
} Point;

const Point PROGMEM BATTERY_INDICATOR_CELLS[] = {
    // left column
    {0, 204},  {0, 187},  {0, 170},  {0, 153},  {0, 136},
    {0, 119},  {0, 102},  {0, 85},  {0, 68},  {0, 51},
    {0, 34},  {0, 17},
    // top row
    {0, 0},  {16, 0},  {32, 0},  {48, 0},  {64, 0},
    {80, 0},  {96, 0},  {112, 0},  {128, 0},  {144, 0},
    {160, 0},
    // right column
    {160, 17},  {160, 34},  {160, 51},  {160, 68},  {160, 85},
    {160, 102},  {160, 119},  {160, 136},  {160, 153},  {160, 170},
    {160, 187},  {160, 204},
};

const Point PROGMEM SPEED_INDICATOR_CELLS[] = {
    // bottom-left row
    {57, 151},  {41, 151}, {25, 151},
    // left column
    {25, 134}, {25, 117}, {25, 100}, {25, 84},
    // top row
    {25, 66}, {41, 66}, {57, 66}, {73, 66}, {90, 66},
    {106, 66}, {122, 66}, {138, 66},
    // right column
    {138, 83}, {138, 100}, {138, 117}, {138, 134},
    // bottom-right row
    {138, 151}, {122, 151}, {106, 151},
};

void DavegaDefaultScreen::reset() {
    _tft->fillRect(0, 0, 200 - 1, 176 - 1,  ILI9341_BLACK);
    _draw_labels();

    // draw FW version
    _tft->setFont(&FreeSans9pt7b);
    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(40, 140);
    _tft->print(_config->fw_version);

    _just_reset = true;
}

void DavegaDefaultScreen::update(t_davega_data* data) {
    char fmt[10];

    if (data->vesc_fault_code != _last_fault_code)
        reset();

    // draw voltage
    if (_config->per_cell_voltage)
        dtostrf(data->voltage / _config->battery_cells, 4, 2, fmt);
    else
        dtostrf(data->voltage, 4, 1, fmt);
    tft_util_draw_number(_tft, fmt, 24, 25,  ILI9341_WHITE,  ILI9341_BLACK, 2, 4);

    // draw mah
    dtostrf(data->mah, 5, 0, fmt);
    tft_util_draw_number(_tft, fmt, 84, 25, progress_to_color(data->mah_reset_progress, _tft),  ILI9341_BLACK, 2, 4);

    // draw trip distance
    dtostrf(convert_distance(data->trip_km, _config->imperial_units), 5, 2, fmt);
    tft_util_draw_number(_tft, fmt, 24, 185, progress_to_color(data->session_reset_progress, _tft),  ILI9341_BLACK, 2, 4);

    // draw total distance
    dtostrf(convert_distance(data->total_km, _config->imperial_units), 4, 0, fmt);
    tft_util_draw_number(_tft, fmt, 98, 185,  ILI9341_WHITE,  ILI9341_BLACK, 2, 4);

    // draw speed
    uint8_t speed = round(convert_speed(data->speed_kph, _config->imperial_units));
    if (speed >= 10)
        tft_util_draw_digit(_tft, speed / 10, 60, 91,  ILI9341_WHITE,  ILI9341_BLACK, 7);
    else
        _tft->fillRect(60, 91, 82, 127,  ILI9341_BLACK);
        tft_util_draw_digit(_tft, speed % 10, 89, 91,  ILI9341_WHITE,  ILI9341_BLACK, 7);

    // draw warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = ILI9341_RED;
        _tft->fillRect(0, 60, 176, 83, bg_color);
        _tft->setFont(&FreeSans12pt7b);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(5, 65);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
    }

    _update_speed_indicator(data->speed_percent, _just_reset);
    _update_battery_indicator(data->battery_percent, _just_reset);

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaDefaultScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(85, 155, 89, 159, color);
    delay(duration_ms);
    _tft->fillRect(85, 155, 89, 159,  ILI9341_BLACK);
}

void DavegaDefaultScreen::_draw_labels() {
    _tft->setFont(&FreeSans9pt7b);

    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(36, 48);
    _tft->print("VOLTS");
    _tft->setCursor(132, 48);
    _tft->print("MAH");

    _tft->setCursor(90, 131);
    _tft->print(_config->imperial_units ? "MPH" : "KPH");

    _tft->setCursor(23, 175);
    _tft->print("TRIP");
    _tft->setCursor(97, 175);
    _tft->print("TOTAL");

    if (_config->imperial_units) {
        _tft->setCursor(50, 208);
        _tft->print("MILES");
        _tft->setCursor(119, 208);
        _tft->print("MILES");
    }
    else {
        _tft->setCursor(70, 208);
        _tft->print("KM");
        _tft->setCursor(139, 208);
        _tft->print("KM");
    }
}

bool DavegaDefaultScreen::_draw_battery_cell(int index, bool filled, bool redraw = false) {
    uint16_t p_word = pgm_read_word_near(BATTERY_INDICATOR_CELLS + index);
    Point *p = (Point *) &p_word;
    if (filled || redraw) {
        uint8_t cell_count = LEN(BATTERY_INDICATOR_CELLS);
        uint8_t green = (uint8_t)(255.0 / (cell_count - 1) * index);
        uint8_t red = 255 - green;
        uint16_t color = _tft->color565(red, green, 0);
        _tft->fillRect(
                p->x, p->y,
                p->x + BATTERY_INDICATOR_CELL_WIDTH, p->y + BATTERY_INDICATOR_CELL_HEIGHT,
                color
        );
    }
    if (!filled) {
        _tft->fillRect(
                p->x + 1, p->y + 1,
                p->x + BATTERY_INDICATOR_CELL_WIDTH - 1, p->y + BATTERY_INDICATOR_CELL_HEIGHT - 1,
                ILI9341_BLACK
        );
    }
}

void DavegaDefaultScreen::_update_battery_indicator(float battery_percent, bool redraw = false) {
    int cells_to_fill = round(battery_percent * LEN(BATTERY_INDICATOR_CELLS));
    if (redraw) {
        for (int i = 0; i < LEN(BATTERY_INDICATOR_CELLS); i++)
            _draw_battery_cell(i, i < cells_to_fill, true);
    }
    else {
        if (cells_to_fill > _battery_cells_filled) {
            for (int i = _battery_cells_filled; i < cells_to_fill; i++)
                _draw_battery_cell(i, true);
        }
        if (cells_to_fill < _battery_cells_filled) {
            for (int i = _battery_cells_filled - 1; i >= cells_to_fill; i--)
                _draw_battery_cell(i, false);
        }
    }
    _battery_cells_filled = cells_to_fill;
}

void DavegaDefaultScreen::_draw_speed_cell(int index, bool filled, bool redraw = false) {
    uint16_t p_word = pgm_read_word_near(SPEED_INDICATOR_CELLS + index);
    Point *p = (Point *) &p_word;
    if (filled || redraw) {
        _tft->fillRect(
                p->x, p->y,
                p->x + SPEED_INDICATOR_CELL_WIDTH, p->y + SPEED_INDICATOR_CELL_HEIGHT,
                _tft->color565(150, 150, 255)
        );
    }
    if (!filled) {
        _tft->fillRect(
                p->x + 1, p->y + 1,
                p->x + SPEED_INDICATOR_CELL_WIDTH - 1, p->y + SPEED_INDICATOR_CELL_HEIGHT - 1,
                 ILI9341_BLACK
        );
    }
}

void DavegaDefaultScreen::_update_speed_indicator(float speed_percent, bool redraw = false) {
    int cells_to_fill = round(speed_percent * LEN(SPEED_INDICATOR_CELLS));
    if (redraw) {
        for (int i = 0; i < LEN(SPEED_INDICATOR_CELLS); i++)
            _draw_speed_cell(i, i < cells_to_fill, true);
    }
    else {
        if (cells_to_fill > _speed_cells_filled) {
            for (int i = _speed_cells_filled; i < cells_to_fill; i++)
                _draw_speed_cell(i, true);
        }
        if (cells_to_fill < _speed_cells_filled) {
            for (int i = _speed_cells_filled - 1; i >= cells_to_fill; i--)
                _draw_speed_cell(i, false);
        }
    }
    _speed_cells_filled = cells_to_fill;
}
