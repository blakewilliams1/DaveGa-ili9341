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

#include "davega_simple_vertical_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include "Adafruit_ILI9341.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

void DavegaSimpleVerticalScreen::reset() {
    _tft->fillRect(0, 0, 220 - 1, 176 - 1, ILI9341_BLACK);

    // labels
    _tft->setFont(&FreeSans9pt7b);
    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(0, 130);
    _tft->print(_config->imperial_units ? "TRIP MI" : "TRIP KM");
    _tft->setCursor(0, 180);
    _tft->print(_config->imperial_units ? "TOTAL MI" : "TOTAL KM");
    _tft->setCursor(110, 130);
    _tft->print("BATTERY %");
    _tft->setCursor(110, 180);
    _tft->print("BATTERY V");

    switch (_primary_item) {
        case SCR_BATTERY_CURRENT:
            _tft->setCursor(82, 0);
            _tft->print("BATTERY A");
            break;
        case SCR_MOTOR_CURRENT:
            _tft->setCursor(96, 0);
            _tft->print("MOTOR A");
            break;
        default:
            _tft->setCursor(122, 0);
            _tft->print(_config->imperial_units ? "MPH" : "KPH");
    }

    // FW version
    _tft->setCursor(0, 0);
    _tft->print(_config->fw_version);

    _just_reset = true;
}

void DavegaSimpleVerticalScreen::update(t_davega_data *data) {
    char fmt[10];

    if (data->vesc_fault_code != _last_fault_code)
        reset();

    // primary display item
    uint8_t value = primary_item_value(_primary_item, data, _config);
    uint16_t color = primary_item_color(_primary_item, data, _config);
    dtostrf(value, 2, 0, fmt);
    tft_util_draw_number(_tft, fmt, 0, 10, color, ILI9341_BLACK, 10, 22);

    // trip distance
    dtostrf(convert_distance(data->trip_km, _config->imperial_units), 5, 2, fmt);
    tft_util_draw_number(_tft, fmt, 0, 140, progress_to_color(data->session_reset_progress, _tft), ILI9341_BLACK, 2, 6);

    // total distance
    format_total_distance(convert_distance(data->total_km, _config->imperial_units), fmt);
    tft_util_draw_number(_tft, fmt, 0, 190, ILI9341_WHITE, ILI9341_BLACK, 2, 6);

    // battery %
    dtostrf(min(100 * data->battery_percent, 99.9), 4, 1, fmt);
    tft_util_draw_number(_tft, fmt, 110, 140, progress_to_color(data->mah_reset_progress, _tft), ILI9341_BLACK, 2, 6);

    // battery voltage
    if (_config->per_cell_voltage)
        dtostrf(data->voltage / _config->battery_cells, 4, 2, fmt);
    else
        dtostrf(data->voltage, 4, 1, fmt);
    tft_util_draw_number(_tft, fmt, 110, 190, ILI9341_WHITE, ILI9341_BLACK, 2, 6);

    // warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = _tft->color565(150, 0, 0);
        _tft->fillRect(0, 180, 176, 220, bg_color);
        _tft->setFont(&FreeSans12pt7b);
        //_tft->setBackgroundColor(bg_color);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(5, 193);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
        //_tft->setBackgroundColor(ILI9341_BLACK);
    }

    _update_battery_indicator(data->battery_percent, _just_reset);

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaSimpleVerticalScreen::_update_battery_indicator(float battery_percent, bool redraw = false) {
    int height = 10;
    int space = 2;
    int cell_count = 10;

    int cells_to_fill = round(battery_percent * cell_count);
    for (int i=0; i<cell_count; i++) {
        bool is_filled = (i < _battery_cells_filled);
        bool should_be_filled = (i < cells_to_fill);
        if (should_be_filled != is_filled || redraw) {
            int y = (cell_count - i - 1) * (height + space) + 1;
            uint8_t green = (uint8_t)(255.0 / (cell_count - 1) * i);
            uint8_t red = 255 - green;
            uint16_t color = _tft->color565(red, green, 0);//setColor(red, green, 0);
            _tft->fillRect(153, y, 175, y + height, color);
            if (!should_be_filled)
                _tft->fillRect(153 + 1, y + 1, 175 - 1, y + height - 1, ILI9341_BLACK);
        }
    }
    _battery_cells_filled = cells_to_fill;
}

void DavegaSimpleVerticalScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(68, 1, 72, 5, color);
    delay(duration_ms);
    _tft->fillRect(68, 1, 72, 5, ILI9341_BLACK);
}
