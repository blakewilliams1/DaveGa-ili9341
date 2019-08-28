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

  // TODO: draw settings buttons

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

    // TODO process touch input.

    return {};
  }

  return {false, false, false};
}
