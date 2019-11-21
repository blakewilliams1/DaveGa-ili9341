#include "led_controller.h"

void LedController::set_animation(animation_type animation) {
  _primary_animation = animation;
}

void LedController::update(t_davega_data* data) {
  update_loop_count++;

  switch(_primary_animation) {
    case FADE:
      fade();
    case SPEED_DASHES:
      speed_dashes(data);
    case RAINBOW_FADE:
      rainbow_fade();
    case RAINBOW_STREAM:
      rainbow_stream();
    case NIGHT_RIDER:
      night_rider();
  }
}

void LedController::speed_dashes(t_davega_data* data) {
  
}

void LedController::fade() {
  int fade_value = (update_loop_count / 10) % (255 * 2);
  if (fade_value > 255) {
    fade_value = 255 - fade_value;
  }

  CHSV hsv;
  hsv.val = fade_value;
  hsv.sat = 255;
  hsv.hue = _primary_color;
  for (int i = 0; i < NUM_LEDS; i++) {
    _leds[i] = hsv;
  }
}

void LedController::rainbow_fade() {
  CHSV hsv;
  hsv.val = 125;//255;
  hsv.sat = 125;//255;
  hsv.hue = (millis() / 50) % 255;

  for (int i = 0; i < NUM_LEDS; i++) {
    _leds[i] = hsv;
  }
}
void LedController::rainbow_stream() {
  CHSV hsv;
  hsv.val = 255;
  hsv.sat = 255;

  unsigned long curr_time = millis();
  
  for (int i = 0; i < NUM_LEDS; i++) {
    // Give initial hue value.
    hsv.hue = (i * 20) % 255;
    // Offset hue value based on current time.
    hsv.hue = (hsv.hue + (curr_time / 50)) % 255;
    _leds[i] = hsv;
  }
}

void LedController::night_rider() {
  int led_number = (millis() / 50) % (NUM_LEDS * 2);
  if (led_number >= NUM_LEDS) {
    led_number = NUM_LEDS - (led_number % NUM_LEDS);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == led_number) {
      _leds[i] = CRGB::Red;
    } else {
      _leds[i] = CRGB::Black;
    }
  }
}
