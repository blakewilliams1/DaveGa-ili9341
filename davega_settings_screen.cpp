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

#include "davega_settings_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include <ILI9341_t3.h> // Hardware-specific library

void DavegaSettingsScreen::reset() {
  _tft->fillScreen(ILI9341_BLACK);

  // Temp units
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(15, 20);
  _tft->print(_config->use_fahrenheit ? "Fahrenheit" : "Celcius");
  _tft->fillRect(20, 35, 60, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(35, 45);
  _tft->print("Temp");
  _tft->setCursor(33, 55);
  _tft->print("Units");

  // Other units
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(100, 20);
  _tft->print(_config->use_fahrenheit ? "Imperial" : "Metric");
  _tft->fillRect(100, 35, 60, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(110, 45);
  _tft->print("Other");
  _tft->setCursor(112, 55);
  _tft->print("Units");


  // Misc read-only settings
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(215, 15);
  _tft->print("Other Settings");
  _tft->setCursor(245, 25);
  _tft->print("something");

  // Navigation label
  _tft->setCursor(15, 100);
  _tft->print("Navigation");

  // Simple horizontal screen navigation
  _tft->fillRect(simple_horizontal_coords.x, simple_horizontal_coords.y, 80, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(simple_horizontal_coords.x + 5, simple_horizontal_coords.y + 10);
  _tft->print("Simple");
  _tft->setCursor(simple_horizontal_coords.x + 5, simple_horizontal_coords.y + 20);
  _tft->print("Horizontal");

  // Simple vertical screen navigation
  _tft->fillRect(simple_vertical_coords.x, simple_vertical_coords.y, 80, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(simple_vertical_coords.x + 5, simple_vertical_coords.y + 10);
  _tft->print("Simple");
  _tft->setCursor(simple_vertical_coords.x + 5, simple_vertical_coords.y + 20);
  _tft->print("Vertical");

  // Realtime graph screen navigation
  _tft->fillRect(realtime_graph_coords.x, realtime_graph_coords.y, 80, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(realtime_graph_coords.x + 5, realtime_graph_coords.y + 10);
  _tft->print("Realtime");
  _tft->setCursor(realtime_graph_coords.x + 5, realtime_graph_coords.y + 20);
  _tft->print("Graph");

  // Default screen navigation
  _tft->fillRect(default_screen_coords.x, default_screen_coords.y, 80, 40, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(default_screen_coords.x + 5, default_screen_coords.y + 10);
  _tft->print("Default");
  _tft->setCursor(default_screen_coords.x + 5, default_screen_coords.y + 20);
  _tft->print("Screen");

  // FW version
  _tft->setCursor(5, 230);
  _tft->print(_config->fw_version);

  _just_reset = true;
}

void DavegaSettingsScreen::update(t_davega_data *data) {
  char fmt[10];

  if (data->vesc_fault_code != _last_fault_code)
    reset();

 

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

void DavegaSettingsScreen::_update_battery_indicator(float battery_percent, bool redraw) {
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(170, 230);
  _tft->print(String(battery_percent) + "%");
  // TODO make cool battery icon.
}

void DavegaSettingsScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
  uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
  _tft->fillRect(50, 230, 6, 6, color);
  delay(duration_ms);
  _tft->fillRect(50, 230, 6, 6, ILI9341_BLACK);
}

t_davega_touch_input DavegaSettingsScreen::handleTouchInput() {
  if (_touch->dataAvailable()) {
    _touch->read();
    int touch_x = _touch->getX();
    int touch_y = _touch->getY();

    // TODO process more touch input.
    // Toggle temp units.
    if (touch_x < 80 && touch_y < 80) {
      
    }

    // Toggle other units.
    if (touch_x >= 80 && touch_x < 160 && touch_y < 80) {
      
    }

    // Location of simple horizontal button with 5px margin.
    if (touch_x > simple_horizontal_coords.x - 5 &&
        touch_x < simple_horizontal_coords.x + 85 &&
        touch_y < simple_horizontal_coords.y - 5 &&
        touch_y < simple_horizontal_coords.y + 45) {
      
    }

    // Location of simple vertical button with 5px margin.
    if (touch_x > simple_vertical_coords.x - 5 &&
        touch_x < simple_vertical_coords.x + 85 &&
        touch_y < simple_vertical_coords.y - 5 &&
        touch_y < simple_vertical_coords.y + 45) {
      
    }

    // Location of realtime graph button with 5px margin.
    if (touch_x > realtime_graph_coords.x - 5 &&
        touch_x < realtime_graph_coords.x + 85 &&
        touch_y < realtime_graph_coords.y - 5 &&
        touch_y < realtime_graph_coords.y + 45) {
      
    }

    // Location of default screen button with 5px margin.
    if (touch_x > default_screen_coords.x - 5 &&
        touch_x < default_screen_coords.x + 85 &&
        touch_y < default_screen_coords.y - 5 &&
        touch_y < default_screen_coords.y + 45) {
      
    }

    return {};
  }

  return {false, false, false};
}
