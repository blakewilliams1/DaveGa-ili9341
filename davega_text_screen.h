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

#ifndef DAVEGA_TEXT_SCREEN_H
#define DAVEGA_TEXT_SCREEN_H

#define MAX_LINE_LENGTH 30

#include <ILI9341_t3.h> // Hardware-specific library
#include "davega_ili9341_screen.h"
#include "davega_config.h"

class DavegaTextScreen: public DavegaILI9341Screen {
public:
    DavegaTextScreen() {
      #ifdef TEXT_SCREEN_ENABLED
      id = TEXT_SCREEN_ENABLED;
      #endif
    }
    void reset();
    void update(t_davega_data* data);
    void heartbeat(uint32_t duration_ms, bool successful_vesc_read);
    uint8_t handleTouchInput(t_davega_button_input* input);

protected:
    void _write_numeric_line(float value, const char* units, const char* label, int lineno, uint16_t color = ILI9341_WHITE);
    void _write_time_line(uint32_t seconds, const char* label, int lineno, uint16_t color = ILI9341_WHITE);
    void _write_text_line(char* value, int lineno, uint16_t color = ILI9341_WHITE);
    void _write_line_buffer(int lineno, uint16_t color = ILI9341_WHITE);

    char _line_buffer[MAX_LINE_LENGTH+1];
};

#endif //DAVEGA_TEXT_SCREEN_H
