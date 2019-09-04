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

    bool use_speed = true;
    bool use_motor_current = false;
    bool use_duty_cycle = false;
    float _max_y_value = 50.0f;
    float _min_y_value = -15.0f;
    uint16_t _x_position = 0;
    uint16_t _graph_colors[5] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_YELLOW, ILI9341_ORANGE};
    // speed, batt current, motor current, motor temp, FET temp 
    t_screen_item graph_value_types[5] =
        {SCR_SPEED, SCR_BATTERY_CURRENT, SCR_MOTOR_CURRENT, SCR_MOTOR_TEMPERATURE, SCR_MOSFET_TEMPERATURE};
    bool selected_graphs[5] = {true, true, false, false, false};
    short graph_lines[305][5] = {{0}};
    bool _selecting_graphs = true;

    float _get_item_value(t_davega_data* data, t_screen_item item);
    void _update_battery_indicator(float battery_percent, bool redraw = false);

private:
    void draw_checkmark(uint16_t x, uint16_t y, bool checked);
};

#endif //DAVEGA_REALTIME_STATS_SCREEN_H
