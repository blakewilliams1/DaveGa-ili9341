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
#include <ILI9341_t3.h> // Hardware-specific library

#define BATTERY_INDICATOR_CELL_WIDTH 19
#define BATTERY_INDICATOR_CELL_HEIGHT 20

#define SPEED_INDICATOR_CELL_WIDTH 16
#define SPEED_INDICATOR_CELL_HEIGHT 17

#define LEN(X) (sizeof(X) / sizeof(X[0]))

const Point PROGMEM BATTERY_INDICATOR_CELLS[] = {
    // left column
    {0, 278},  {0, 255},  {0, 232},  {0, 209},  {0, 185},
    {0, 162},  {0, 139},  {0, 116},  {0, 93},  {0, 70},
    {0, 46},  {0, 23},
    // top row
    {0, 0},  {22, 0},  {44, 0},  {65, 0},  {87, 0},
    {109, 0},  {131, 0},  {153, 0},  {175, 0},  {196, 0},
    {218, 0},
    // right column
    {218, 23},  {218, 46},  {218, 70},  {218, 93},  {218, 116},
    {218, 139},  {218, 162},  {218, 185},  {218, 209},  {218, 232},
    {218, 255},  {218, 278},
};

const Point PROGMEM SPEED_INDICATOR_CELLS[] = {
    // bottom-left row
    {77, 205},  {55, 205}, {33, 205},
    // left column
    {33, 182}, {33, 159}, {33, 135}, {33, 114},
    // top row
    {33, 89}, {55, 89}, {77, 89}, {99, 89}, {122, 89},
    {144, 89}, {165, 89}, {187, 89},
    // right column
    {187, 112}, {187, 135}, {187, 159}, {187, 182},
    // bottom-right row
    {187, 205}, {165, 205}, {144, 205},
};

void DavegaDefaultScreen::reset() {
    _tft->fillScreen(ILI9341_BLACK);
    _draw_labels();

    // draw FW version
    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(55, 191);
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
    tft_util_draw_number(_tft, fmt, 33, 34,  ILI9341_WHITE,  ILI9341_BLACK, 2, 5);

    // draw mah
    dtostrf(data->mah, 5, 0, fmt);
    tft_util_draw_number(_tft, fmt, 115, 34, progress_to_color(data->mah_reset_progress, _tft),  ILI9341_BLACK, 2, 5);

    // draw trip distance
    dtostrf(convert_distance(data->trip_km, _config->imperial_units), 5, 2, fmt);
    tft_util_draw_number(_tft, fmt, 33, 252, progress_to_color(data->session_reset_progress, _tft),  ILI9341_BLACK, 2, 5);

    // draw total distance
    dtostrf(convert_distance(data->total_km, _config->imperial_units), 4, 0, fmt);
    tft_util_draw_number(_tft, fmt, 134, 252,  ILI9341_WHITE,  ILI9341_BLACK, 2, 5);

    // draw speed
    uint8_t speed = round(convert_speed(data->speed_kph, _config->imperial_units));
    if (speed >= 10)
        tft_util_draw_digit(_tft, speed / 10, 82, 124,  ILI9341_WHITE,  ILI9341_BLACK, 10);
    else
        _tft->fillRect(82, 124, 30, 49,  ILI9341_BLACK);
        tft_util_draw_digit(_tft, speed % 10, 121, 124,  ILI9341_WHITE,  ILI9341_BLACK, 10);

    // draw warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = ILI9341_RED;
        _tft->fillScreen(bg_color);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(7, 89);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
    }

    _update_speed_indicator(data->speed_percent, _just_reset);
    _update_battery_indicator(data->battery_percent, _just_reset);

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaDefaultScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(116, 211, 6, 6, color);
    delay(duration_ms);
    _tft->fillRect(116, 211, 6, 6,  ILI9341_BLACK);
}

void DavegaDefaultScreen::_draw_labels() {

    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(49, 67);
    _tft->print("VOLTS");
    _tft->setCursor(180, 67);
    _tft->print("MAH");

    _tft->setCursor(123, 179);
    _tft->print(_config->imperial_units ? "MPH" : "KPH");

    _tft->setCursor(31, 239);
    _tft->print("TRIP");
    _tft->setCursor(132, 239);
    _tft->print("TOTAL");

    _tft->fillRect(5, 295, 110, 20, ILI9341_WHITE);
    _tft->setTextColor(ILI9341_BLACK);
    _tft->setCursor(20, 300);
    _tft->print("Flip Screen");

    _tft->fillRect(125, 295, 110, 20, ILI9341_WHITE);
    _tft->setTextColor(ILI9341_BLACK);
    _tft->setCursor(160, 300);
    _tft->print("Settings");

    _tft->setTextColor(ILI9341_WHITE);
    if (_config->imperial_units) {
        _tft->setCursor(68, 284);
        _tft->print("MILES");
        _tft->setCursor(162, 284);
        _tft->print("MILES");
    }
    else {
        _tft->setCursor(95, 284);
        _tft->print("KM");
        _tft->setCursor(190, 284);
        _tft->print("KM");
    }
}

void DavegaDefaultScreen::_draw_battery_cell(int index, bool filled, bool redraw) {
    uint16_t p_word = pgm_read_word_near(BATTERY_INDICATOR_CELLS + index);
    Point *p = (Point *) &p_word;
    if (filled || redraw) {
        uint8_t cell_count = LEN(BATTERY_INDICATOR_CELLS);
        uint8_t green = (uint8_t)(255.0 / (cell_count - 1) * index);
        uint8_t red = 255 - green;
        uint16_t color = _tft->color565(red, green, 0);
        _tft->fillRect(
                p->x, p->y,
                BATTERY_INDICATOR_CELL_WIDTH, BATTERY_INDICATOR_CELL_HEIGHT,
                color
        );
    }
    if (!filled) {
        _tft->fillRect(
                p->x + 1, p->y + 1,
                BATTERY_INDICATOR_CELL_WIDTH - 2, BATTERY_INDICATOR_CELL_HEIGHT - 2,
                ILI9341_BLACK
        );
    }
}

void DavegaDefaultScreen::_update_battery_indicator(float battery_percent, bool redraw) {
    unsigned int cells_to_fill = round(max(battery_percent, 0) * LEN(BATTERY_INDICATOR_CELLS));
    if (redraw) {
        for (unsigned int i = 0; i < LEN(BATTERY_INDICATOR_CELLS); i++)
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

void DavegaDefaultScreen::_draw_speed_cell(int index, bool filled, bool redraw) {
    uint16_t p_word = pgm_read_word_near(SPEED_INDICATOR_CELLS + index);
    Point *p = (Point *) &p_word;
    if (filled || redraw) {
        _tft->fillRect(
                p->x, p->y,
                SPEED_INDICATOR_CELL_WIDTH, SPEED_INDICATOR_CELL_HEIGHT,
                _tft->color565(150, 150, 255)
        );
    }
    if (!filled) {
        _tft->fillRect(
                p->x + 1, p->y + 1,
                SPEED_INDICATOR_CELL_WIDTH - 2, SPEED_INDICATOR_CELL_HEIGHT - 2,
                ILI9341_BLACK
        );
    }
}

void DavegaDefaultScreen::_update_speed_indicator(float speed_percent, bool redraw) {
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

uint8_t DavegaDefaultScreen::handleTouchInput(t_davega_button_input* input) {
  // Flip screen 180 degrees.
  if (input->touch_x <= 120 && input->touch_y > 290) {
    _config->orientation = (_config->orientation + 2) % 4;
    _tft->setRotation(_config->orientation);
    reset();
  }

  // Navigate to settings menu.
  if (input->touch_x > 120 && input->touch_y > 290) {
    #ifdef SETTINGS_SCREEN_ENABLED
    _config->orientation = LANDSCAPE_ORIENTATION;
    _tft->setRotation(_config->orientation);
    return SETTINGS_SCREEN_ENABLED;
    #endif
  }
  return 0;
}
