#include "davega_led_controller_screen.h"
#include "davega_screen.h"
#include "davega_util.h"
#include "davega_config.h"
#include "vesc_comm.h"
#include "tft_util.h"
#include <ILI9341_t3.h> // Hardware-specific library

void DavegaLedControllerScreen::reset() {
    _tft->fillScreen(ILI9341_BLACK);

    // Draw the touch input buttons. Only needs to happen once, they don't update look.
    _tft->setTextColor(ILI9341_BLACK);
    _tft->fillRect(2, 220, 101, 20, ILI9341_WHITE);
    _tft->setCursor(30, 227);
    _tft->print("Flip Screen");

    _tft->fillRect(109, 220, 101, 20, ILI9341_WHITE);
    _tft->setCursor(137, 227);
    _tft->print("BUTTON 2");

    _tft->fillRect(216, 220, 101, 20, ILI9341_WHITE);
    _tft->setCursor(244, 227);
    _tft->print("Settings");

    // FW version
    _tft->setCursor(0, 115);
    _tft->print(_config->fw_version);

    _just_reset = true;
}

void DavegaLedControllerScreen::update(t_davega_data *data) {
    char fmt[10];

    if (data->vesc_fault_code != _last_fault_code)
        reset();
  
    // warning
    if (data->vesc_fault_code != FAULT_CODE_NONE) {
        uint16_t bg_color = ILI9341_RED;
        _tft->fillScreen(bg_color);
        _tft->setTextColor(ILI9341_BLACK);
        _tft->setCursor(7, 220);
        _tft->print(vesc_fault_code_to_string(data->vesc_fault_code));
    } else {
      _update_battery_indicator(data->battery_percent, _just_reset);
    }

    _last_fault_code = data->vesc_fault_code;
    _just_reset = false;
}

void DavegaLedControllerScreen::_update_battery_indicator(float battery_percent, bool redraw) {
  _tft->fillRect(50, 10, 45, 30, ILI9341_BLACK);
  _tft->setTextColor(ILI9341_WHITE);
  _tft->setCursor(60, 230);
  _tft->print(String(battery_percent * 100) + "%");
}

void DavegaLedControllerScreen::heartbeat(uint32_t duration_ms, bool successful_vesc_read) {
    uint16_t color = successful_vesc_read ? ILI9341_GREEN : ILI9341_RED;
    _tft->fillRect(91, 5, 6, 6, color);
    delay(duration_ms);
    _tft->fillRect(91, 5, 6, 6, ILI9341_BLACK);
}

uint8_t DavegaLedControllerScreen::handleTouchInput(t_davega_button_input* input) {
  // Rotate the screen 180 degrees.
  if (input->touch_x < 103 && input->touch_y > 210) {
    _config->orientation = (_config->orientation + 2) % 4;
    _tft->setRotation(_config->orientation);
    reset();
  }

  // TODO: change condition when buttons are added.
  // Increment the selected animation type.
  if (false) {
    // TODO: verify LEN() is returning correct value
    selected_animation = (selected_animation + 1) % LEN(_animation_types);
    //_tft->fillRect(100, 100, 80, 20, ILI9341_BLACK);
    _tft->setCursor(100, 100);
    _tft->print(_animation_types[selected_animation].label);
    _led_controller->set_animation(_animation_types[selected_animation].type);
  }
  // Decrement the selected animation type.
  if (false) {
    selected_animation--;
    if (selected_animation < 0) {
      selected_animation = LEN(_animation_types) - 1;
    }
    //_tft->fillRect(100, 100, 80, 20, ILI9341_BLACK);
    _tft->setCursor(100, 100);
    _tft->print(_animation_types[selected_animation].label);
    _led_controller->set_animation(_animation_types[selected_animation].type);
  }

  if (input->touch_x > 215 && input->touch_y > 210) {
    #ifdef SETTINGS_SCREEN_ENABLED
    _config->orientation = LANDSCAPE_ORIENTATION;
    _tft->setRotation(_config->orientation);
    return SETTINGS_SCREEN_ENABLED;
    #endif
  }
  return 0;
}
