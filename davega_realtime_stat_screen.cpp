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
#include "Adafruit_ILI9341.h"

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
  /*_tft->fillRect(200, 220, 100, 15, ILI9341_WHITE);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(210, 223);
  _tft->print("Change graph");*/

  _just_reset = true;
}

void DavegaRealtimeStatScreen::update(t_davega_data *data) {
  char fmt[10];

  if (data->vesc_fault_code != _last_fault_code)
      reset();

  int y_axis = (_max_y_value * 220) / (_max_y_value - _min_y_value);
  for (int i = 0; i < 5; i++) {
    t_screen_item curr_item = graph_items[i];
    float item_value = _get_item_value(data, curr_item);

    /*_tft->fillRect(90, 110, 50, 40, ILI9341_BLACK);
    _tft->setCursor(100, 125);
    _tft->setTextColor(ILI9341_WHITE);
    _tft->print(item_value);*/

    // Track min/max values, redraw on change.
    if (item_value > _max_y_value) {
      _max_y_value = item_value;
      // reset();
    }
    if (item_value < _min_y_value) {
      _min_y_value = item_value;
    //  reset();
    }

    int color = _graph_colors[i];
    // Y coord is derived from an affine transformation.
    int y_coord = ((item_value - _min_y_value) * -200) / (_max_y_value - _min_y_value) + 200;

    // Don't draw pixel if it's going to mar the min/max values, or over UI.
    if ((_x_position < 245 || y_coord > 30) && y_coord != y_axis && y_coord <= 220 ) {
      _tft->drawPixel(_x_position + 15, y_coord, ILI9341_RED);
      _tft->drawPixel(_x_position + 15, y_coord, ILI9341_RED);
    }
  }

  _x_position++;
  if (_x_position > 305) _x_position = 0;

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
  // TODO: Remove when done testing
  //return ((float)random(0, 3000) / 100.0f) - 15.0f;
  float avg_speed_kph = 0;
  switch (item) {
    case SCR_MOSFET_TEMPERATURE:
      return convert_temperature(data->mosfet_celsius, _config->use_fahrenheit);
    case SCR_MOTOR_TEMPERATURE:
      return convert_temperature(data->motor_celsius, _config->use_fahrenheit);
    case SCR_MOTOR_CURRENT:
      return data->motor_amps;
    case SCR_BATTERY_CURRENT:
      return data->battery_amps;
    case SCR_DUTY_CYCLE:
      return data->duty_cycle * 100.0f;
    case SCR_TOTAL_VOLTAGE:
      return data->voltage;
    case SCR_BATTERY_CAPACITY_PERCENT:
      return data->battery_percent * 100.0f;
    case SCR_SPEED:
      return convert_distance(data->speed_kph, _config->imperial_units);
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
   _x_position++;
   if (_x_position >= 305){ _x_position = 0;}
}

t_davega_touch_input DavegaRealtimeStatScreen::handleTouchInput() {
  if (_touch->dataAvailable()) {
    _touch->read();
    int touch_x = _touch->getX();
    int touch_y = _touch->getY();

    // TODO process touch input.

    return {};
  }

  return {false, false, false};
}
