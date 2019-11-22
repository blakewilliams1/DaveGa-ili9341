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
#include "davega_config.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include <ILI9341_t3.h> // Hardware-specific library

void DavegaSettingsScreen::reset() {
  _tft->fillScreen(ILI9341_BLACK);
  buttonCursor = 0;
  updateHighlighting(temp_units_coords, temp_units_coords, _tft);

  // Temp units
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(temp_units_coords.x, temp_units_coords.y - 15);
  _tft->print(_config->use_fahrenheit ? "Fahrenheit" : "Celcius");
  _tft->fillRect(
    temp_units_coords.x,
    temp_units_coords.y,
    temp_units_coords.width,
    temp_units_coords.height,
    ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(temp_units_coords.x + 10, temp_units_coords.y + 10);
  _tft->print("Temp");
  _tft->setCursor(temp_units_coords.x + 13, temp_units_coords.y + 20);
  _tft->print("Units");

  // Other units
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(other_units_coords.x, other_units_coords.y - 15);
  _tft->print(_config->imperial_units ? "Imperial" : "Metric");
  _tft->fillRect(
    other_units_coords.x,
    other_units_coords.y,
    other_units_coords.width,
    other_units_coords.height,
    ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(other_units_coords.x + 10, other_units_coords.y + 10);
  _tft->print("Other");
  _tft->setCursor(other_units_coords.x + 12, other_units_coords.y  + 20);
  _tft->print("Units");

  // Primary menu item
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(primary_value_coords.x, primary_value_coords.y - 15);
  char const* primary_item_label;
  switch(_config->primary_screen_item) {
    case SCR_MOTOR_CURRENT:
      primary_item_label = "Motor Amp";
      break;
    case SCR_BATTERY_CURRENT:
      primary_item_label = "Bat Amp";
      break;
    case SCR_SPEED:
      primary_item_label = "Speed";
      break;
    default: primary_item_label = "?";
  }
  _tft->print(primary_item_label);
  _tft->fillRect(
    primary_value_coords.x,
    primary_value_coords.y,
    primary_value_coords.width,
    primary_value_coords.height,
    ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(primary_value_coords.x + 5, primary_value_coords.y + 10);
  _tft->print("Primary");
  _tft->setCursor(primary_value_coords.x + 8, primary_value_coords.y + 20);
  _tft->print("Value");

  // Screen Orientation
  _tft->setTextColor(ILI9341_WHITE);
  _tft->fillRect(
    rotate_screen_coords.x,
    rotate_screen_coords.y,
    rotate_screen_coords.width,
    rotate_screen_coords.height,
    ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(rotate_screen_coords.x + 10, rotate_screen_coords.y + 10);
  _tft->print("Flip");
  _tft->setCursor(rotate_screen_coords.x + 10, rotate_screen_coords.y + 20);
  _tft->print("Screen");

  // Navigation label
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(20, 100);
  _tft->print("Navigation");

  // Simple horizontal screen navigation
  #ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
  _tft->fillRect(
    simple_horizontal_coords.x,
    simple_horizontal_coords.y,
    simple_horizontal_coords.width,
    simple_horizontal_coords.height, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(simple_horizontal_coords.x + 5, simple_horizontal_coords.y + 10);
  _tft->print("Simple");
  _tft->setCursor(simple_horizontal_coords.x + 5, simple_horizontal_coords.y + 20);
  _tft->print("Horizontal");
  #endif

  // Simple vertical screen navigation
  #ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
  _tft->fillRect(
    simple_vertical_coords.x,
    simple_vertical_coords.y,
    simple_vertical_coords.width,
    simple_vertical_coords.height, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(simple_vertical_coords.x + 5, simple_vertical_coords.y + 10);
  _tft->print("Simple");
  _tft->setCursor(simple_vertical_coords.x + 5, simple_vertical_coords.y + 20);
  _tft->print("Vertical");
  #endif

  // Text screen navigation
  #ifdef TEXT_SCREEN_ENABLED
  _tft->fillRect(
    text_screen_coords.x,
    text_screen_coords.y,
    text_screen_coords.width,
    text_screen_coords.height, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(text_screen_coords.x + 5, text_screen_coords.y + 10);
  _tft->print("Text");
  _tft->setCursor(text_screen_coords.x + 5, text_screen_coords.y + 20);
  _tft->print("Screen");
  #endif

  // Realtime graph screen navigation
  #ifdef REALTIME_STATS_SCREEN_ENABLED
  _tft->fillRect(
    realtime_graph_coords.x,
    realtime_graph_coords.y,
    realtime_graph_coords.width,
    realtime_graph_coords.height, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(realtime_graph_coords.x + 5, realtime_graph_coords.y + 10);
  _tft->print("Realtime");
  _tft->setCursor(realtime_graph_coords.x + 5, realtime_graph_coords.y + 20);
  _tft->print("Graph");
  #endif

  // Default screen navigation
  #ifdef DEFAULT_SCREEN_ENABLED
  _tft->fillRect(
    default_screen_coords.x,
    default_screen_coords.y,
    default_screen_coords.width,
    default_screen_coords.height, ILI9341_WHITE); 
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(default_screen_coords.x + 5, default_screen_coords.y + 10);
  _tft->print("Default");
  _tft->setCursor(default_screen_coords.x + 5, default_screen_coords.y + 20);
  _tft->print("Screen");
  #endif

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

uint8_t DavegaSettingsScreen::handleTouchInput(t_davega_button_input* input) {
  // Allows reset to run after all important logic finshes.
  bool trigger_redraw = false;
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
        _config->use_fahrenheit = !_config->use_fahrenheit;
        trigger_redraw = true;
        break;
      case 1: 
        _config->imperial_units = !_config->imperial_units;
        trigger_redraw = true;
        break;
      case 2: 
        trigger_redraw = true;
        _primary_options_index++;
        if (_primary_options_index > 2) _primary_options_index = 0;
        _config->primary_screen_item = primary_options[_primary_options_index];
      case 3: 
        _config->orientation = (_config->orientation + 2) % 4;
        _tft->setRotation(_config->orientation);
        reset();
        break;
      case 4: 
        #ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
        return SIMPLE_HORIZONTAL_SCREEN_ENABLED;
        #endif
      case 5: 
      // Consider removing this.
        #ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
        _config->orientation = PORTRAIT_ORIENTATION;
        _tft->setRotation(_config->orientation);
        return SIMPLE_VERTICAL_SCREEN_ENABLED;
        #endif
        break;
      case 6: 
        #ifdef TEXT_SCREEN_ENABLED
        return TEXT_SCREEN_ENABLED;
        #endif
        break;
      case 7: 
        #ifdef REALTIME_STATS_SCREEN_ENABLED
        return REALTIME_STATS_SCREEN_ENABLED;
        #endif
        break;
      case 8: 
        #ifdef DEFAULT_SCREEN_ENABLED
        _config->orientation = PORTRAIT_ORIENTATION;
        _tft->setRotation(_config->orientation);
        return DEFAULT_SCREEN_ENABLED;
        #endif
        break;
    }
  }

  // Toggle temp units.
  if (input->touch_x > temp_units_coords.x - 10 &&
      input->touch_x < temp_units_coords.x + 60 &&
      input->touch_y < temp_units_coords.y + 45) {
    _config->use_fahrenheit = !_config->use_fahrenheit;
    trigger_redraw = true;
  }

  // Toggle other units.
  if (input->touch_x > other_units_coords.x - 10 &&
      input->touch_x < other_units_coords.x + 60 &&
      input->touch_y < other_units_coords.y + 45) {
    _config->imperial_units = !_config->imperial_units;
    trigger_redraw = true;
  }

  // Increment primary element choice.
  if (input->touch_x > primary_value_coords.x - 10 &&
      input->touch_x < primary_value_coords.x + 60 &&
      input->touch_y < primary_value_coords.y + 45) {
    trigger_redraw = true;
    _primary_options_index++;
    if (_primary_options_index > 2) _primary_options_index = 0;
    _config->primary_screen_item = primary_options[_primary_options_index];
  }

  // Flip screen 180 degrees.
  if (input->touch_x > rotate_screen_coords.x - 10 &&
      input->touch_x < rotate_screen_coords.x + 60 &&
      input->touch_y < rotate_screen_coords.y + 45) {
    _config->orientation = (_config->orientation + 2) % 4;
    _tft->setRotation(_config->orientation);
    reset();
  }

  if (trigger_redraw) { 
    reset();
  }

  // Location of simple horizontal button with 5px margin.
  if (input->touch_x > simple_horizontal_coords.x - 5 &&
      input->touch_x < simple_horizontal_coords.x + 85 &&
      input->touch_y > simple_horizontal_coords.y - 5 &&
      input->touch_y < simple_horizontal_coords.y + 45) {
    #ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
    return SIMPLE_HORIZONTAL_SCREEN_ENABLED;
    #endif
  }

  // Location of simple vertical button with 5px margin.
  if (input->touch_x > simple_vertical_coords.x - 5 &&
      input->touch_x < simple_vertical_coords.x + 85 &&
      input->touch_y > simple_vertical_coords.y - 5 &&
      input->touch_y < simple_vertical_coords.y + 45) {
    #ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
    _config->orientation = PORTRAIT_ORIENTATION;
    _tft->setRotation(_config->orientation);
    return SIMPLE_VERTICAL_SCREEN_ENABLED;
    #endif
  }

  // Location of text info button with 5px margin.
  if (input->touch_x > text_screen_coords.x - 5 &&
      input->touch_x < text_screen_coords.x + 85 &&
      input->touch_y > text_screen_coords.y - 5 &&
      input->touch_y < text_screen_coords.y + 45) {
    #ifdef TEXT_SCREEN_ENABLED
    return TEXT_SCREEN_ENABLED;
    #endif
  }

  // Location of realtime graph button with 5px margin.
  if (input->touch_x > realtime_graph_coords.x - 5 &&
      input->touch_x < realtime_graph_coords.x + 85 &&
      input->touch_y > realtime_graph_coords.y - 5 &&
      input->touch_y < realtime_graph_coords.y + 45) {
    #ifdef REALTIME_STATS_SCREEN_ENABLED
    return REALTIME_STATS_SCREEN_ENABLED;
    #endif
  }

  // Location of default screen button with 5px margin.
  if (input->touch_x > default_screen_coords.x - 5 &&
      input->touch_x < default_screen_coords.x + 85 &&
      input->touch_y > default_screen_coords.y - 5 &&
      input->touch_y < default_screen_coords.y + 45) {
    #ifdef DEFAULT_SCREEN_ENABLED
    _config->orientation = PORTRAIT_ORIENTATION;
    _tft->setRotation(_config->orientation);
    return DEFAULT_SCREEN_ENABLED;
    #endif
  }

  return 0;
}
