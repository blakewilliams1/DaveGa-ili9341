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
  _selecting_graphs = false;
  _tft->fillScreen(ILI9341_BLACK);
	buttonCursor = 0;

  updateHighlighting(settings_button, settings_button, _tft);
  // Draw axis info
  draw_axis_indicator();

  // FW version
  _tft->setCursor(5, 230);
  _tft->print(_config->fw_version);

  // Draw buttons
  _tft->fillRect(100, 220, 100, 18, ILI9341_WHITE);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->setCursor(110, 227);
  _tft->print("Change graph");
  _tft->fillRect(216, 220, 100, 18, ILI9341_WHITE);
  _tft->setCursor(244, 227);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->print("Settings");

  _just_reset = true;
}

void DavegaRealtimeStatScreen::update(t_davega_data *data) {
  if (data->vesc_fault_code != _last_fault_code)
    reset();

  if (!_selecting_graphs) {

    // Draw axis info
    draw_axis_indicator();

    int y_axis = (_max_y_value * 220) / (_max_y_value - _min_y_value);
    float item_value = 0;
    short prev_value = 0;
    bool should_redraw_graph = false;
    // Iterate over graphs and draw the selected ones.
    for (int i = 0; i < 6; i++) {
      if (!graph_elements[i].visible) continue;

      item_value = _get_item_value(data, graph_elements[i].item_type);
      prev_value = _x_position > 0 ? graph_lines[_x_position - 1][i] / 100 : 0;
      graph_lines[_x_position][i] = (short)item_value * 100;
      // Y coords derived from affine transformations.
      int curr_y_coord = ((item_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
      int prev_y_coord = ((prev_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
      _tft->drawLine(_x_position + 15, curr_y_coord, _x_position + 14, prev_y_coord, graph_elements[i].color);

      if (item_value > _max_y_value && item_value < 2000) {
        _max_y_value = item_value;
        should_redraw_graph = true;
      }
      if (item_value < _min_y_value && item_value > -2000) {
        _min_y_value = item_value;
        should_redraw_graph = true;
      }
    }

    // Clean upcoming graph space and remove past wipe's graph lines
    int y_top = 0;
    // Prevents smearing of the Y axis info.
    if (_x_position >= 240) {
      y_top += 35;
    }
    _tft->drawLine(_x_position + 16, y_top, _x_position + 16, y_axis - 1, ILI9341_BLACK);
    _tft->drawLine(_x_position + 16, y_axis + 1, _x_position + 16, 220, ILI9341_BLACK);

    _x_position++;
    if (_x_position >= 305) {
      _x_position = 0;
      _max_y_value = 30.0f;
      _min_y_value = -15.0f;
    }

    // Dynamically adjust the graph to fit the data needed.
    if (should_redraw_graph) { 
      _tft->fillScreen(ILI9341_BLACK);
      draw_axis_indicator();
      for(int x = 0; x < _x_position; x++) {
        for (int i = 0; i < 6; i++) {
          if (!graph_elements[i].visible) continue;
          item_value = graph_lines[x][i] / 100;
          prev_value = x > 0 ? graph_lines[x - 1][i] / 100 : 0;
          // Y coords derived from affine transformations.
          int curr_y_coord = ((item_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
          int prev_y_coord = ((prev_value - _min_y_value) * -220) / (_max_y_value - _min_y_value) + 220;
          _tft->drawLine(x + 15, curr_y_coord, x + 14, prev_y_coord, graph_elements[i].color);
        }
      }
    }
  }

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

void DavegaRealtimeStatScreen::draw_axis_indicator() {
  _tft->drawRect(0, 0, 14, 220, ILI9341_BLACK);
  // X axis location is dynamic based on max and min values to display
  int x_axis = (_max_y_value * 220) / (_max_y_value - _min_y_value);
  _tft->drawLine(15, x_axis, 320, x_axis, ILI9341_WHITE);
  // Y axis. Last 20 pixels on bottom of screen are for text and buttons.
  _tft->drawLine(15, 0, 15, 220, ILI9341_WHITE);

  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(5, x_axis - 4);
  _tft->print("0");
  _tft->setCursor(245, 5);
  _tft->print("Y Axis");
  _tft->setCursor(245, 15);
  _tft->print("Max: " + String(_max_y_value));
  _tft->setCursor(245, 25);
  _tft->print("Min: " + String(_min_y_value));
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
    case SCR_DUTY_CYCLE:
      return data->duty_cycle * 100.0f;
    default: break;
  }

  return 0;
}

void DavegaRealtimeStatScreen::draw_checkmark(uint16_t x, uint16_t y, bool checked) {
  _tft->drawRect(x, y, 15, 15, ILI9341_WHITE);
  if (checked) {
    _tft->drawLine(x + 4, y + 14, x + 16, y - 2, ILI9341_GREEN);
    _tft->drawLine(x + 5, y + 15, x + 17, y - 3, ILI9341_GREEN);
    _tft->drawLine(x + 4, y + 14, x, y + 7, ILI9341_GREEN);
    _tft->drawLine(x + 5, y + 15, x - 1, y + 8, ILI9341_GREEN);
  }
}

void DavegaRealtimeStatScreen::_update_battery_indicator(float battery_percent, bool redraw) {
  _tft->fillRect(50, 223, 45, 30, ILI9341_BLACK);
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(60, 230);
  _tft->print(String(battery_percent * 100) + "%");
}

void DavegaRealtimeStatScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
  uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
  _tft->fillRect(35, 230, 6, 6, color);
  delay(duration_ms);
  _tft->fillRect(35, 230, 6, 6, ILI9341_BLACK);
}

void DavegaRealtimeStatScreen::draw_graph_menu() {
  _tft->fillScreen(ILI9341_BLACK);
  // Draw the graph choices
  for (uint8_t i = 0; i < 6; i++) {
    uint16_t x = 30 + 160 * (i % 2);
    uint16_t y = 30 + 60 * (i / 2);

    draw_checkmark(x, y, graph_elements[i].visible);
    _tft->setTextColor(graph_elements[i].color);
    _tft->setCursor(x + 25, y + 5);
    _tft->print(graph_elements[i].label);
  }

  _tft->fillRect(190, 220, 126, 17, ILI9341_WHITE);
  _tft->setCursor(210, 225);
  _tft->setTextColor(ILI9341_BLACK);
  _tft->print("Back to graph");
}

uint8_t DavegaRealtimeStatScreen::handleTouchInput(t_davega_button_input* input) {
  // Debug code
  /*_tft->fillRect(120, 100, 100, 50, ILI9341_BLUE);
  _tft->setCursor(130, 110);
  _tft->print(input->button_1_pressed);
  _tft->setCursor(130, 120);
  _tft->print(input->button_3_pressed);
  _tft->setCursor(130, 130);
  _tft->print(buttonCursor);
  if (input->touch_x > 0 && input->touch_y > 0) {
    _last_press = millis();
  }*/
  // end debug code

  long since_last_switch = millis() - _last_screen_switch;
	if (_selecting_graphs) {
		if (input->button_1_pressed) {
			Button oldButton = graph_setting_buttons[buttonCursor];
			buttonCursor = (buttonCursor + 1) % LEN(graph_setting_buttons);
			updateHighlighting(oldButton, graph_setting_buttons[buttonCursor], _tft);
		} else if (input->button_3_pressed) {
			Button oldButton = graph_setting_buttons[buttonCursor];
			buttonCursor =
			    (buttonCursor + LEN(graph_setting_buttons) - 1) % LEN(graph_setting_buttons);
			updateHighlighting(oldButton, graph_setting_buttons[buttonCursor], _tft);
		} else if (input->button_2_pressed) {
			// toggle graph
			if (buttonCursor <= 5) {
				graph_elements[buttonCursor].visible = !graph_elements[buttonCursor].visible;
				_last_press = millis();
				draw_graph_menu();
			} else {
				// Go back to graphs.
				_selecting_graphs = false;
				_last_screen_switch = millis();
				reset();
        buttonCursor = 0;
  			updateHighlighting(buttons[buttonCursor], buttons[buttonCursor], _tft);
			}
		}
			// Iterate through the locations of all the graph type buttons.
    for (uint8_t i = 0; i < 6; i++) {
      uint16_t x = 30 + 160 * (i % 2);
      uint16_t y = 30 + 60 * (i / 2);
      if (input->touch_x > x - 20 &&
          input->touch_x < x + 120 &&
          input->touch_y > y - 20 &&
          input->touch_y < y + 35 &&
          since_last_switch > 1000) {
        graph_elements[i].visible = !graph_elements[i].visible;
        _last_press = millis();
        draw_graph_menu();
      }
    }

    // Touched the 'back to graph' button.
    if (input->touch_x > 210 && input->touch_y > 210 && since_last_switch > 1000) {
      _selecting_graphs = false;
      _last_screen_switch = millis();
      reset();
    }
  } else {
		// Not in selecting graphs menu
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
					_selecting_graphs = true;
					_last_screen_switch = millis();
					draw_graph_menu();
					buttonCursor = 0;
          updateHighlighting(speed_button, speed_button, _tft);
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
    // Navigate to settings menu.
    if (input->touch_x > 215 && input->touch_y > 210 && since_last_switch > 1000) {
      #ifdef SETTINGS_SCREEN_ENABLED
      _config->orientation += 2;
      _tft->setRotation(_config->orientation);
      return SETTINGS_SCREEN_ENABLED;
      #endif
    }

    // Opens the graph selection menu from graph view.
    if (input->touch_x < 215 && input->touch_y > 220) {
      _selecting_graphs = true;
      _last_screen_switch = millis();
      draw_graph_menu();
    }
  }

  return 0;
}
