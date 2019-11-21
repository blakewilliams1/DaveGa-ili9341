#ifndef LED_CONTROLLER
#define LED_CONTROLLER

#include <FastLED.h>
#include "davega_data.h"

#define NUM_LEDS 20

#define DATA_PIN 27 // 14 on Teensy 4.0

typedef enum {
  PULSE,
  FADE,
  SPEED_DASHES,
  RAINBOW_FADE,
  RAINBOW_STREAM,
  NIGHT_RIDER,
} animation_type;

class LedController {
  public:
  CRGB _leds[NUM_LEDS];

  LedController() {  
    LEDS.addLeds<WS2812, DATA_PIN, RGB>(_leds, NUM_LEDS);
  }

  //void init(t_davega_screen_config* config) override;
  void update(t_davega_data* data);
  void set_animation(animation_type type);

  private:
  uint64_t update_loop_count = 0;
  animation_type _primary_animation = RAINBOW_FADE;
  uint16_t _primary_color;
  uint16_t _secondary_color;

  void fade();
  void pulse();
  void speed_dashes(t_davega_data* data);
  void rainbow_fade();
  void rainbow_stream();
  void night_rider();
};

#endif //LED_CONTROLLER
