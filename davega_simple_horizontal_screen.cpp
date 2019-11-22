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

#include "davega_simple_horizontal_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "davega_config.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include <ILI9341_t3.h> // Hardware-specific library

void DavegaSimpleHorizontalScreen::reset() {
    _tft->fillScreen(ILI9341_BLACK);

    updateHighlighting(flip_screen_button, flip_screen_button, _tft);

    _tft->setTextColor(ILI9341_WHITE);
    _tft->setCursor(274, 42);
    _tft->print(_config->imperial_units ? "TRIP MI" : "TRIP KM");
    _tft->setCursor(266, 103);
    _tft->print(_config->imperial_units ? "TOTAL MI" : "TOTAL KM");
    _tft->setCursor(258, 163);
    _tft->print("BATTERY V");

    switch (_config->primary_screen_item) {
        case SCR_BATTERY_CURRENT:
            _tft->setCursor(179, 167);
            _tft->print("BATTERY A");
            break;
        case SCR_MOTOR_CURRENT:
            _tft->setCursor(193, 167);
            _tft->print("MOTOR A");
            break;
        default:
            _tft->setCursor(180, 167);
            _tft->print(_config->imperial_units ? "MPH" : "KPH");
    }

    // Draw the touch input buttons. Only needs to happen once, they don't update look.
    _tft->setTextColor(ILI9341_BLACK);
    _tft->fillRect(
				flip_screen_button.x,
				flip_screen_button.y,
				flip_screen_button.width,
				flip_screen_button.height, ILI9341_WHITE);
    _tft->setCursor(30, 227);
    _tft->print("Flip Screen");

    _tft->fillRect(109, 220, 101, 20, ILI9341_WHITE);
    _tft->setCursor(137, 227);
    _tft->print("BUTTON 2");

    _tft->fillRect(
				settings_button.x,
				settings_button.y,
				settings_button.width,
				settings_button.height, ILI9341_WHITE);
    _tft->setCursor(244, 227);
    _tft->print("Settings");

    // FW version
    _tft->setCursor(0, 115);
    _tft->print(_config->fw_version);

    _just_reset = true;
}

void DavegaSimpleHorizontalScreen::update(t_davega_data *data) {
    char fmt[10];

    if (data->vesc_fault_code != _last_fault_code)
        reset();

    // primary display item
    uint8_t value = primary_item_value(_config->primary_screen_item, data, _config);
    uint16_t color = primary_item_color(_config->primary_screen_item, data, _config);
    dtostrf(value, 2, 0, fmt);
    tft_util_draw_number(_tft, fmt, 0, 0, color, ILI9341_BLACK, 7, 32);

    // trip distance
    dtostrf(convert_distance(data->trip_km, _config->imperial_units), 5, 2, fmt);
    tft_util_draw_number(_tft, fmt, 200, 0, progress_to_color(data->session_reset_progress, _tft), ILI9341_BLACK, 3, 8);

    // total distance
    format_total_distance(convert_distance(data->total_km, _config->imperial_units), fmt);
    tft_util_draw_number(_tft, fmt, 200, 61, ILI9341_WHITE, ILI9341_BLACK, 3, 8);

    // battery voltage
    if (_config->per_cell_voltage) {
      dtostrf(data->voltage / _config->battery_cells, 4, 2, fmt);
    } else {
      dtostrf(data->voltage, 4, 1, fmt);
    }
    tft_util_draw_number(_tft, fmt, 222, 121, progress_to_color(data->mah_reset_progress, _tft), ILI9341_BLACK, 3, 8);

    // warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = ILI9341_RED;
        _tft->fillScreen(bg_color);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(7, 220);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
    } else {
      _update_battery_indicator(data->battery_percent, _just_reset);
    }

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaSimpleHorizontalScreen::_update_battery_indicator(float battery_percent, bool redraw) {
    float width = 25;
    float space = 4;
    float cell_count = 11;

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
            _tft->fillRect(x, 179, width, 35, color);
            if (!should_be_filled)
                _tft->fillRect(x + 1, 180, width - 2, 33, ILI9341_BLACK);
        }
    }
    _battery_cells_filled = cells_to_fill;
}

void DavegaSimpleHorizontalScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(91, 167, 6, 6, color);
    delay(duration_ms);
    _tft->fillRect(91, 167, 6, 6, ILI9341_BLACK);
}

uint8_t DavegaSimpleHorizontalScreen::handleTouchInput(t_davega_button_input* input) {
  if (input->button_1_pressed) {
    Button oldButton = buttons[buttonCursor];
    buttonCursor = (buttonCursor + 1) % LEN(buttons);
    updateHighlighting(oldButton, buttons[buttonCursor], _tft);
  } else if (input->button_3_pressed) {
    Button oldButton = buttons[buttonCursor];
    buttonCursor = (buttonCursor + LEN(buttons) - 1) % LEN(buttons);
    updateHighlighting(oldButton, buttons[buttonCursor], _tft);
  } else if (input->button_2_pressed) {
    switch(buttonCursor) {
      case 0: 
        _config->orientation = (_config->orientation + 2) % 4;
        _tft->setRotation(_config->orientation);
        reset();
        break;
      case 1: 
        #ifdef SETTINGS_SCREEN_ENABLED
        _config->orientation = LANDSCAPE_ORIENTATION;
        _tft->setRotation(_config->orientation);
        return SETTINGS_SCREEN_ENABLED;
        #endif
        break;
    }
  }

  // Rotate the screen 180 degrees.
  if (input->touch_x < 103 && input->touch_y > 210) {
    _config->orientation = (_config->orientation + 2) % 4;
    _tft->setRotation(_config->orientation);
    reset();
  }

  if (input->touch_x > 215 && input->touch_y > 210) {
    #ifdef SETTINGS_SCREEN_ENABLED
    _config->orientation = LANDSCAPE_ORIENTATION;
    _tft->setRotation(_config->orientation);
    return SETTINGS_SCREEN_ENABLED;
    #endif
  }
  return 0;
}
