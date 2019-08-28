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

#ifndef DAVEGA_SETTINGS_SCREEN_H
#define DAVEGA_SETTINGS_SCREEN_H

#include <ILI9341_t3.h> // Hardware-specific library
#include "davega_ili9341_screen.h"
#include "davega_simple_screen.h"

class DavegaSettingsScreen: public DavegaILI9341Screen {
public:
    DavegaSettingsScreen() {    }

    void reset();
    void update(t_davega_data* data);
    void heartbeat(uint32_t duration_ms, bool successful_vesc_read);
    t_davega_touch_input handleTouchInput();

protected:
    // Have we just reset the screen? Unset by the first update() call.
    bool _just_reset = false;

    vesc_comm_fault_code _last_fault_code = FAULT_CODE_NONE;

    void _update_battery_indicator(float battery_percent, bool redraw = false);
};

#endif //DAVEGA_SETTINGS_SCREEN_H
