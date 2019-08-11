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
//#include <Fonts/FreeSans9pt7b.h>
//#include <Fonts/FreeSans12pt7b.h>

void DavegaSimpleVerticalScreen::reset() {
    _tft->fillScreen(ILI9341_BLACK);

    // labels
    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(0, 189);
    _tft->print(_config->imperial_units ? "TRIP MI" : "TRIP KM");
    _tft->setCursor(0, 252);
    _tft->print(_config->imperial_units ? "TOTAL MI" : "TOTAL KM");
    _tft->setCursor(150, 189);
    _tft->print("BATTERY %");
    _tft->setCursor(150, 252);
    _tft->print("BATTERY V");

    switch (_primary_item) {
        case SCR_BATTERY_CURRENT:
            _tft->setCursor(145, 0);
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
    tft_util_draw_number(_tft, fmt, 0, 13, color, ILI9341_BLACK, 13, 32);

    // trip distance
    dtostrf(convert_distance(data->trip_km, _config->imperial_units), 5, 2, fmt);
    tft_util_draw_number(_tft, fmt, 0, 204, progress_to_color(data->session_reset_progress, _tft), ILI9341_BLACK, 3, 8);

    // total distance
    format_total_distance(convert_distance(data->total_km, _config->imperial_units), fmt);
    tft_util_draw_number(_tft, fmt, 0, 266, ILI9341_WHITE, ILI9341_BLACK, 3, 8);

    // battery %
    dtostrf(min(100 * data->battery_percent, 99.9), 4, 1, fmt);
    tft_util_draw_number(_tft, fmt, 145, 204, progress_to_color(data->mah_reset_progress, _tft), ILI9341_BLACK,3, 8);

    // battery voltage
    if (_config->per_cell_voltage)
        dtostrf(data->voltage / _config->battery_cells, 4, 2, fmt);
    else
        dtostrf(data->voltage, 4, 1, fmt);
    tft_util_draw_number(_tft, fmt, 145, 266, ILI9341_WHITE, ILI9341_BLACK, 3, 8);

    // warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = _tft->color565(150, 0, 0);
        _tft->fillScreen(bg_color);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(5, 193);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
    }

    _update_battery_indicator(data->battery_percent, _just_reset);

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaSimpleVerticalScreen::_update_battery_indicator(float battery_percent, bool redraw = false) {
    int height = 13;
    int space = 3;
    int cell_count = 10;

    int cells_to_fill = round(battery_percent * cell_count);
    for (int i=0; i<cell_count; i++) {
        bool is_filled = (i < _battery_cells_filled);
        bool should_be_filled = (i < cells_to_fill);
        if (should_be_filled != is_filled || redraw) {
            int x = (cell_count - i - 1) * (width + space);
            uint8_t red = 255;
            if (i > cell_count/2) {
              int curr_cell =  (int)cell_count/2;
              red -= (255 / curr_cell) * (i - curr_cell);
            }

            uint8_t green = 255;
            if (i < cell_count/2) {
              int curr_cell =  (int)cell_count/2;
              green -= (255 / curr_cell) * (curr_cell - i);
            }
            uint16_t color = _tft->color565(red, green, 0);
            _tft->fillRect(207, y, 31, height, color);
            if (!should_be_filled)
                _tft->fillRect(208, y + 1, 29, height - 2, ILI9341_BLACK);
        }
    }
    _battery_cells_filled = cells_to_fill;
}

void DavegaSimpleVerticalScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(99, 2, 6, 6, color);
    delay(duration_ms);
    _tft->fillRect(99, 2, 6, 6, ILI9341_BLACK);
}

t_davega_touch_input DavegaSimpleVerticalScreen::handleTouchInput() {
  if (_touch->dataAvailable()) {
    _touch->read();
    int touch_x = _touch->getX();
    int touch_y = _touch->getY();
    
  }

  return {};
}
