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

#ifndef DAVEGA_REALTIME_STATS_SCREEN_H
#define DAVEGA_REALTIME_STATS_SCREEN_H

#include <ILI9341_t3.h> // Hardware-specific library
#include "davega_ili9341_screen.h"
#include "davega_simple_screen.h"
#include "davega_config.h"

typedef struct {
  bool visible;
  t_screen_item item_type;
  const char* label;
  uint16_t color;
} GraphElement;

class DavegaRealtimeStatScreen: public DavegaILI9341Screen {
public:
    DavegaRealtimeStatScreen() {
      #ifdef REALTIME_STATS_SCREEN_ENABLED
      id = REALTIME_STATS_SCREEN_ENABLED;
      #endif
    }

    void reset();
    void update(t_davega_data* data);
    void heartbeat(uint32_t duration_ms, bool successful_vesc_read);
    uint8_t handleTouchInput(t_davega_button_input* input);

protected:
    // Have we just reset the screen? Unset by the first update() call.
    bool _just_reset = false;

    vesc_comm_fault_code _last_fault_code = FAULT_CODE_NONE;

    long _last_screen_switch = 0;
    long _last_press = 0;
    float _max_y_value = 30.0f;
    float _min_y_value = -15.0f;
    uint16_t _x_position = 0;
    GraphElement graph_elements[6] = {
      {true, SCR_SPEED, "Speed", ILI9341_RED},
      {true, SCR_BATTERY_CURRENT, "Bat Current", ILI9341_GREEN},
      {false, SCR_MOTOR_CURRENT, "Motor Current", ILI9341_BLUE},
      {false, SCR_MOTOR_TEMPERATURE, "Motor Temp", ILI9341_YELLOW},
      {false, SCR_DUTY_CYCLE, "Duty Cycle", ILI9341_PURPLE},
      {false, SCR_MOSFET_TEMPERATURE, "FET Temp", ILI9341_CYAN},
    };
    short graph_lines[305][5] = {{0}};
    bool _selecting_graphs = true;

    float _get_item_value(t_davega_data* data, t_screen_item item);
    void _update_battery_indicator(float battery_percent, bool redraw = false);

    Button change_graph_button = {100, 220, 100, 18};
    Button settings_button = {216, 220, 100, 18};
   // Array of available buttons and a cursor to iterate them with buttons.
    Button buttons[2] = {
      change_graph_button,
      settings_button,
    };

    Button speed_button = {15, 15, 120, 60};
    Button battery_amp_button = {175, 15, 120, 60};
    Button motor_amp_button = {15, 75, 120, 60};
    Button motor_temp_button = {175, 75, 120, 60};
    Button duty_cycle_button = {15, 135, 120, 60};
    Button fet_temp_button = {175, 135, 120, 60};
    Button back_button = {190, 220, 126, 17};
    Button graph_setting_buttons[7] = {
			speed_button,
			battery_amp_button,
			motor_amp_button,
			motor_temp_button,
			duty_cycle_button,
			fet_temp_button,
      back_button,
    };

    uint8_t buttonCursor = 0;
private:
    void draw_graph_menu();
    void draw_axis_indicator();
    void draw_checkmark(uint16_t x, uint16_t y, bool checked);
};

#endif //DAVEGA_REALTIME_STATS_SCREEN_H
