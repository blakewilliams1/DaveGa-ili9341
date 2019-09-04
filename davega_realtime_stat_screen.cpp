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

#include "davega_realtime_stat_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include <ILI9341_t3.h> // Hardware-specific library

void DavegaRealtimeStatScreen::reset() {
  _tft->fillScreen(ILI9341_BLACK);

  // X axis location is dynamic based on max and min values to display
  int x_axis = (_max_y_value * 220) / (_max_y_value - _min_y_value);
  _tft->drawLine(15, x_axis, 320, x_axis, ILI9341_WHITE);
  // Y axis. Last 20 pixels on bottom of screen are for text and buttons.
  _tft->drawLine(15, 0, 15, 220, ILI9341_WHITE);

  // Draw axis info
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(5, x_axis - 4);
  _tft->print("0");
  _tft->setCursor(245, 5);
  _tft->print("Y Axis");
  _tft->setCursor(245, 15);
  _tft->print("Max: " + String(_max_y_value));
  _tft->setCursor(245, 25);
  _tft->print("Min: " + String(_min_y_value));

  // FW version
  _tft->setCursor(5, 230);
  _tft->print(_config->fw_version);

  // Draw buttons
  _tft->fillRect(200, 220, 100, 15, ILI9341_WHITE);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(210, 223);
  _tft->print("Change graph");
  _tft->fillRect(216, 220, 101, 20, ILI9341_WHITE);
  _tft->setCursor(244, 227);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->print("Settings");

  _just_reset = true;
}

void DavegaRealtimeStatScreen::update(t_davega_data *data) {
  if (data->vesc_fault_code != _last_fault_code)
    reset();

  int y_axis = (_max_y_value * 220) / (_max_y_value - _min_y_value);
  float item_value = 0;
  short prev_value = 0;
  // Iterate over graphs and draw the selected ones.
  for (int i = 0; i < 5; i++) {
    if (!selected_graphs[i]) continue;

    item_value = _get_item_value(data, graph_value_types[i]);
    prev_value = _x_position > 0 ? graph_lines[_x_position - 1][i] / 100 : 0;
    graph_lines[_x_position][i] = (short)item_value * 100;
    // Y coords derived from affine transformations.
    int curr_y_coord = ((item_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
    int prev_y_coord = ((prev_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
    _tft->drawLine(_x_position + 15, curr_y_coord, _x_position + 14, prev_y_coord, _graph_colors[i]);
  }

  // Clean upcoming graph space and remove past wipe's graph lines
  int y_top = 0;
  // Prevents smearing of the Y axis info.
  if (_x_position >= 245) {
    y_top += 35;
  }
  _tft->drawLine(_x_position + 16, y_top, _x_position + 16, y_axis - 1, ILI9341_BLACK);
  _tft->drawLine(_x_position + 16, y_axis + 1, _x_position + 16, 220, ILI9341_BLACK);

  _x_position++;
  if (_x_position >= 305) _x_position = 0;

  // warning
  if (data->vesc_fault_code != FAULT_CODE_NONE) {
    _tft->fillScreen(ILI9341_RED);
    _tft->setTextColor(ILI9341_BLACK);
    _tft->setCursor(7, 220);
    _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
  } else {
    _update_battery_indicator(data->battery_percent, _just_reset);
  }

  _last_fault_code = data->vesc_fault_code;
  _just_reset = false;
}

float DavegaRealtimeStatScreen::_get_item_value(t_davega_data* data, t_screen_item item) {
  switch (item) {
    case SCR_SPEED:
      return convert_speed(data->speed_kph, _config->imperial_units);
    case SCR_MOSFET_TEMPERATURE:
      return convert_temperature(data->mosfet_celsius, _config->use_fahrenheit);
    case SCR_MOTOR_TEMPERATURE:
      return convert_temperature(data->motor_celsius, _config->use_fahrenheit);
    case SCR_BATTERY_CURRENT:
      return data->battery_amps;
    case SCR_MOTOR_CURRENT:
      return data->motor_amps;
    case SCR_TOTAL_VOLTAGE:
      return data->voltage;
    case SCR_BATTERY_CAPACITY_PERCENT:
      return data->battery_percent * 100.0f;
    default: break;
  }

  return 0;
}

void DavegaRealtimeStatScreen::_update_battery_indicator(float battery_percent, bool redraw) {
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(170, 230);
  _tft->print(String(battery_percent) + "%");
  // TODO make cool battery icon.
}

void DavegaRealtimeStatScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
  uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
  _tft->fillRect(50, 230, 6, 6, color);
  delay(duration_ms);
  _tft->fillRect(50, 230, 6, 6, ILI9341_BLACK);
}

uint8_t DavegaRealtimeStatScreen::handleTouchInput(t_davega_button_input* input) {
  // Navigate to settings menu
  if (input->touch_x > 215 && input->touch_y > 220) {
    #ifdef SETTINGS_SCREEN_ENABLED
    return SETTINGS_SCREEN_ENABLED;
    #endif
  }

  return 0;
}
