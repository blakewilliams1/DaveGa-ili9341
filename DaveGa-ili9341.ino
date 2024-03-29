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

#include "davega_config.h"
#include "davega_eeprom.h"
#include "davega_data.h"
#include "davega_util.h"
#include "davega_screen.h"
#include "vesc_comm.h"
#include <URTouch.h>
#include <URTouchCD.h>

#define REVISION_ID "$Id: ca658c2601bcf00dee70bada1de15351e591c9a4 $"
#define FW_VERSION "master"

#define DEBUG
#ifdef DEBUG
#define D(x) Serial.println(x)
#else
#define D(x)
#endif

// 15, 16, 17 on Teensy 4.0
#define BUTTON_1_PIN A3
#define BUTTON_2_PIN A2
#define BUTTON_3_PIN A4

#ifdef FOCBOX_UNITY
#include "vesc_comm_unity.h"
VescCommUnity vesc_comm = VescCommUnity();
#else
#include "vesc_comm_standard.h"
VescCommStandard vesc_comm = VescCommStandard();
#endif

#ifdef REALTIME_STATS_SCREEN_ENABLED
#include "davega_realtime_stat_screen.h"
DavegaRealtimeStatScreen davega_realtime_stats_screen = DavegaRealtimeStatScreen();
#endif
#ifdef SETTINGS_SCREEN_ENABLED
#include "davega_settings_screen.h"
DavegaSettingsScreen davega_settings_screen = DavegaSettingsScreen();
#endif
#ifdef VERTICAL_SETTINGS_SCREEN_ENABLED
#include "davega_vertical_settings_screen.h"
DavegaVerticalSettingsScreen davega_vertical_settings_screen = DavegaVerticalSettingsScreen();
#endif
#ifdef DEFAULT_SCREEN_ENABLED
#include "davega_default_screen.h"
DavegaDefaultScreen davega_default_screen = DavegaDefaultScreen();
#endif
#ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
#include "davega_simple_horizontal_screen.h"
DavegaSimpleHorizontalScreen davega_simple_horizontal_screen = DavegaSimpleHorizontalScreen();
#endif
#ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
#include "davega_simple_vertical_screen.h"
DavegaSimpleVerticalScreen davega_simple_vertical_screen = DavegaSimpleVerticalScreen(SCR_SPEED);
#endif
#ifdef LED_CONTROLLER_SCREEN_ENABLED
#include "led_controller.h"
LedController* led_controller = new LedController();
#include "davega_led_controller_screen.h"
DavegaLedControllerScreen davega_led_controller_screen = DavegaLedControllerScreen(led_controller);
#endif
#ifdef TEXT_SCREEN_ENABLED
#include "davega_text_screen.h"
DavegaTextScreen davega_text_screen = DavegaTextScreen();
#endif

DavegaScreen* davega_screens[] = {
#ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
  &davega_simple_horizontal_screen,
#endif
#ifdef REALTIME_STATS_SCREEN_ENABLED
  &davega_realtime_stats_screen,
#endif
#ifdef LED_CONTROLLER_SCREEN_ENABLED
  &davega_led_controller_screen,
#endif
#ifdef DEFAULT_SCREEN_ENABLED
  &davega_default_screen,
#endif
#ifdef VERTICAL_SETTINGS_SCREEN_ENABLED
  &davega_vertical_settings_screen,
#endif
#ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
  &davega_simple_vertical_screen,
#endif
#ifdef SETTINGS_SCREEN_ENABLED
  &davega_settings_screen,
#endif
#ifdef TEXT_SCREEN_ENABLED
  &davega_text_screen,
#endif
};

char* versionNumber = "1.0";
t_davega_screen_config screen_config = {
  // TODO: fix issue with calling make_fw_version.
  versionNumber,//make_fw_version(FW_VERSION, REVISION_ID),
  IMPERIAL_UNITS,
  USE_FAHRENHEIT,
  SHOW_AVG_CELL_VOLTAGE,
  BATTERY_S,
  TEXT_SCREEN_BIG_FONT,
  SCR_SPEED,
  SCREEN_ORIENTATION
};

int current_screen_index = 0;
DavegaScreen* scr;

const float discharge_ticks[] = DISCHARGE_TICKS;

t_davega_data data;
t_davega_session_data session_data;
int32_t initial_mah_spent;
int32_t initial_trip_meters;
int32_t initial_total_meters;
int32_t initial_millis_elapsed;
int32_t last_millis = 0;
int32_t last_eeprom_written_total_meters;
int32_t last_eeprom_update_on_stop;

int32_t last_rpm;
uint32_t button_1_last_up_time = 0;
uint32_t button_2_last_up_time = 0;
uint32_t button_3_last_up_time = 0;
bool is_touched_prev = false;

int32_t rotations_to_meters(int32_t rotations) {
  float gear_ratio = float(WHEEL_PULLEY_TEETH) / float(MOTOR_PULLEY_TEETH);
  return (rotations / MOTOR_POLE_PAIRS / gear_ratio) * WHEEL_DIAMETER_MM * PI / 1000;
}
#define TCLK 19
#define TCS 20
#define TDIN 21
#define DOUT 22
#define IRQ 23

URTouch touch_screen(IRQ, DOUT, TDIN, TCS, TCLK);

float erpm_to_kph(uint32_t erpm) {
  float km_per_minute = rotations_to_meters(erpm) / 1000.0;
  return km_per_minute * 60.0;
}

float voltage_to_percent(float voltage) {
  if (voltage < discharge_ticks[0])
    return 0.0;
  for (unsigned int i = 1; i < LEN(discharge_ticks); i++) {
    float cur_voltage = discharge_ticks[i];
    if (voltage < cur_voltage) {
      float prev_voltage = discharge_ticks[i - 1];
      float interval_perc = (voltage - prev_voltage) / (cur_voltage - prev_voltage);
      float low = 1.0 * (i - 1) / (LEN(discharge_ticks) - 1);
      float high = 1.0 * i / (LEN(discharge_ticks) - 1);
      return low + (high - low) * interval_perc;
    }
  }
  return 1.0;
}

bool was_battery_charged(float last_volts, float current_volts) {
  return (current_volts - last_volts) / current_volts > FULL_CHARGE_MIN_INCREASE;
}

bool is_battery_full(float current_volts) {
  float max_volts = discharge_ticks[LEN(discharge_ticks) - 1];
  return current_volts / max_volts > FULL_CHARGE_THRESHOLD;
}

void setup() {
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);

#ifdef DEBUG
  Serial.begin(115200);
#endif
  touch_screen.InitTouch();
  touch_screen.setPrecision(PREC_MEDIUM);

  vesc_comm.init(115200);

  if (!eeprom_is_initialized(EEPROM_MAGIC_VALUE)) {
    eeprom_initialize(EEPROM_MAGIC_VALUE);
    eeprom_write_volts(EEPROM_INIT_VALUE_VOLTS);
    eeprom_write_mah_spent(EEPROM_INIT_VALUE_MAH_SPENT);
    eeprom_write_total_distance(EEPROM_INIT_VALUE_TOTAL_DISTANCE);
    session_data.max_speed_kph = EEPROM_INIT_VALUE_MAX_SPEED;
    session_data.millis_elapsed = EEPROM_INIT_VALUE_MILLIS_ELAPSED;
    session_data.millis_riding = EEPROM_INIT_VALUE_MILLIS_RIDING;
    session_data.min_voltage = EEPROM_INIT_VALUE_MIN_VOLTAGE;
    session_data.trip_meters = EEPROM_INIT_VALUE_TRIP_DISTANCE;
    eeprom_write_session_data(session_data);
  }

  for (unsigned int i = 0; i < LEN(davega_screens); i++)
    davega_screens[i]->init(&screen_config);
  scr = davega_screens[current_screen_index];

  session_data = eeprom_read_session_data();
  data.voltage = eeprom_read_volts();
  data.mah = BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY - eeprom_read_mah_spent();
  data.trip_km = session_data.trip_meters / 1000.0;
  data.total_km = eeprom_read_total_distance() / 1000.0;
  data.session = &session_data;

  scr->reset();
  scr->update(&data);

  // TODO: Remove this after testing.
  //while(true)scr->update(&data);

  vesc_comm.fetch_packet();
  while (!vesc_comm.is_expected_packet()) {
    scr->heartbeat(UPDATE_DELAY, false);
    vesc_comm.fetch_packet();
  }

  float last_volts = eeprom_read_volts();
  float current_volts = vesc_comm.get_voltage();
  if (was_battery_charged(last_volts, current_volts)) {
    // reset mAh spent
    if (is_battery_full(current_volts)) {
      eeprom_write_mah_spent(0);
    }
    else {
      float soc = voltage_to_percent(current_volts);
      uint16_t mah_spent = (uint16_t) (BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY * (1 - soc));
      eeprom_write_mah_spent(mah_spent);
    }
    eeprom_write_volts(current_volts);
  }

  initial_mah_spent = eeprom_read_mah_spent();
  initial_trip_meters = session_data.trip_meters;
  initial_total_meters = eeprom_read_total_distance();
  initial_millis_elapsed = session_data.millis_elapsed;

  last_eeprom_written_total_meters = initial_total_meters;
  last_eeprom_update_on_stop = millis();

  // Subtract current VESC values, which could be non-zero in case the display
  // got reset without resetting the VESC as well. The invariant is:
  //   current value = initial value + VESC value
  // and that works correctly with the default initial values in case the VESC values
  // start from 0. If that's not the case though we need to lower the initial values.
  int32_t vesc_mah_spent = VESC_COUNT * (vesc_comm.get_amphours_discharged() -
                                         vesc_comm.get_amphours_charged());
  initial_mah_spent -= vesc_mah_spent;
  int32_t tachometer = rotations_to_meters(vesc_comm.get_tachometer() / 6);
  initial_trip_meters -= tachometer;
  initial_total_meters -= tachometer;
}

void set_screen(int screen_id) {
  for (unsigned int i = 0; i < LEN(davega_screens); i++) {
    if (screen_id == davega_screens[i]->id) {
      scr = davega_screens[i];
      scr->reset();
    }
  }
}

void navigateScreens(int navigationCommand) {
  switch (navigationCommand) {
    #ifdef SIMPLE_HORIZONTAL_SCREEN_ENABLED
    case SIMPLE_HORIZONTAL_SCREEN_ENABLED:
    set_screen(SIMPLE_HORIZONTAL_SCREEN_ENABLED);
    break;
    #endif
    #ifdef SIMPLE_VERTICAL_SCREEN_ENABLED
    case SIMPLE_VERTICAL_SCREEN_ENABLED:
    set_screen(SIMPLE_VERTICAL_SCREEN_ENABLED);
    break;
    #endif
    #ifdef TEXT_SCREEN_ENABLED
    case TEXT_SCREEN_ENABLED:
    set_screen(TEXT_SCREEN_ENABLED);
    break;
    #endif
    #ifdef REALTIME_STATS_SCREEN_ENABLED
    case REALTIME_STATS_SCREEN_ENABLED:
    set_screen(REALTIME_STATS_SCREEN_ENABLED);
    break;
    #endif
    #ifdef DEFAULT_SCREEN_ENABLED
    case DEFAULT_SCREEN_ENABLED:
    set_screen(DEFAULT_SCREEN_ENABLED);
    break;
    #endif
    #ifdef SETTINGS_SCREEN_ENABLED
    case SETTINGS_SCREEN_ENABLED:
    set_screen(SETTINGS_SCREEN_ENABLED);
    break;
    #endif
    #ifdef VERTICAL_SETTINGS_SCREEN_ENABLED
    case VERTICAL_SETTINGS_SCREEN_ENABLED:
    set_screen(VERTICAL_SETTINGS_SCREEN_ENABLED);
    break;
    #endif
    #ifdef LED_CONTROLLER_SCREEN_ENABLED
    case LED_CONTROLLER_SCREEN_ENABLED:
    set_screen(LED_CONTROLLER_SCREEN_ENABLED);
    break;
    #endif
  }
}

bool button_1_pressed_prev = false;
bool button_2_pressed_prev = false;
bool button_3_pressed_prev = false;

void loop() {
  bool button_1_pressed = false;
  bool button_2_pressed = false;
  bool button_3_pressed = false;
  if (digitalRead(BUTTON_3_PIN) == LOW) {
    button_3_pressed = true;
    if (!button_3_pressed_prev) {
      button_3_last_up_time = millis();
    }
  }

  if (digitalRead(BUTTON_1_PIN) == LOW) {
    button_1_pressed = true;
    if (!button_1_pressed_prev) {
      button_1_last_up_time = millis();
    }
  }

  if (digitalRead(BUTTON_2_PIN) == LOW) {
    button_2_pressed = true;
    if (!button_2_pressed_prev) {
      button_2_last_up_time = millis();
    }
  }



  // Process touch input and send to the screen for interpretting.
  int touch_x = 0;
  int touch_y = 0;
  bool is_touched = false;
  if (touch_screen.dataAvailable()) {
    touch_screen.read();
    // Touch screen library gives coordinates from lower right corner
    // of screen, so fix it depending on orientation.
    if (screen_config.orientation % 2 == LANDSCAPE_ORIENTATION) {
      if (screen_config.orientation == 3) {
        touch_x = (int16_t)touch_screen.getX();
        touch_y = (int16_t)touch_screen.getY();
      } else {
        touch_x = 320 - (int16_t)touch_screen.getX();
        touch_y = 240 - (int16_t)touch_screen.getY();
      }
      // There was a false reading with coordinate outside of the screen.
      if (touch_x > 320 || touch_y > 240) {
        touch_x = 0;
        touch_y = 0;
      } else {
        is_touched = true;
      }
    } else {
      if (screen_config.orientation == 0) {
        touch_x = (int16_t)touch_screen.getY();
        touch_y = 320 - (int16_t)touch_screen.getX();
      } else {
        touch_x = 240 - (int16_t)touch_screen.getY();
        touch_y = (int16_t)touch_screen.getX();
      }
      // There was a false reading with coordinate outside of the screen.
      if (touch_x > 240 || touch_y > 320) {
        touch_x = 0;
        touch_y = 0;
      } else {
        is_touched = true;
      }
    }
  }

  // Process current screen's touch input and hardware button input.
  t_davega_button_input input = {
    button_1_pressed && !button_1_pressed_prev,
    button_2_pressed && !button_2_pressed_prev,
    button_3_pressed && !button_3_pressed_prev,
    is_touched && !is_touched_prev,
    touch_x,
    touch_y
  };
  int navigationCommand = scr->handleTouchInput(&input);
  navigateScreens(navigationCommand);

  is_touched_prev = is_touched;
  vesc_comm.fetch_packet();

  if (!vesc_comm.is_expected_packet()) {
    scr->heartbeat(UPDATE_DELAY, false);
    return;
  }

  data.mosfet_celsius = vesc_comm.get_temp_mosfet();
  data.motor_celsius = vesc_comm.get_temp_motor();
  data.motor_amps = vesc_comm.get_motor_current();
  data.battery_amps = vesc_comm.get_battery_current() * VESC_COUNT;
  data.duty_cycle = vesc_comm.get_duty_cycle();
  data.vesc_fault_code = vesc_comm.get_fault_code();
  data.voltage = vesc_comm.get_voltage();

  // TODO: DRY
  int32_t vesc_mah_spent = VESC_COUNT * (vesc_comm.get_amphours_discharged() -
                                         vesc_comm.get_amphours_charged());
  int32_t mah_spent = initial_mah_spent + vesc_mah_spent;
  int32_t mah = BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY - mah_spent;

  uint32_t button_1_down_elapsed = button_1_pressed ? millis() - button_1_last_up_time : 0;
  uint32_t button_2_down_elapsed = button_2_pressed ? millis() - button_2_last_up_time : 0;
  uint32_t button_3_down_elapsed = button_3_pressed ? millis() - button_3_last_up_time : 0;
  /*if (button_2_down_elapsed > COUNTER_RESET_TIME) {
    // reset coulomb counter
    mah = voltage_to_percent(data.voltage) * BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY;
    mah_spent = BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY - mah;
    eeprom_write_mah_spent(mah_spent);
    initial_mah_spent = mah_spent - vesc_mah_spent;
  }*/

  data.mah = mah;

  // dim mAh if the counter is about to be reset
  data.mah_reset_progress = min(1.0 * button_2_down_elapsed / COUNTER_RESET_TIME, 1.0);

  int32_t rpm = vesc_comm.get_rpm();
  data.speed_kph = max(erpm_to_kph(rpm), 0);

  int32_t tachometer = rotations_to_meters(vesc_comm.get_tachometer() / 6);

  /*if (button_1_down_elapsed > COUNTER_RESET_TIME) {
    // reset session
    session_data.trip_meters = 0;
    session_data.max_speed_kph = 0;
    session_data.millis_elapsed = 0;
    session_data.millis_riding = 0;
    session_data.min_voltage = data.voltage;
    eeprom_write_session_data(session_data);
    initial_trip_meters = -tachometer;
    initial_millis_elapsed = -millis();
  }*/

  session_data.trip_meters = initial_trip_meters + tachometer;
  int32_t total_meters = initial_total_meters + tachometer;

  // dim trip distance if it's about to be reset
  data.session_reset_progress = min(1.0 * button_1_down_elapsed / COUNTER_RESET_TIME, 1.0);

  data.trip_km = session_data.trip_meters / 1000.0;
  data.total_km = total_meters / 1000.0;

  data.speed_percent = 1.0 * data.speed_kph / MAX_SPEED_KPH;

  data.mah_percent = 1.0 * mah / (BATTERY_MAX_MAH * BATTERY_USABLE_CAPACITY);
  data.voltage_percent = voltage_to_percent(data.voltage);
  data.battery_percent = VOLTAGE_WEIGHT * data.voltage_percent + (1.0 - VOLTAGE_WEIGHT) * data.mah_percent;

  // extreme values
  if (data.speed_kph > session_data.max_speed_kph)
    session_data.max_speed_kph = data.speed_kph;

  if (data.voltage < session_data.min_voltage)
    session_data.min_voltage = data.voltage;

  // time elapsed
  session_data.millis_elapsed = initial_millis_elapsed + millis();

  if (rpm > 0)
    session_data.millis_riding += millis() - last_millis;
  last_millis = millis();

  // update EEPROM
  bool came_to_stop = (last_rpm != 0 && rpm == 0);
  bool traveled_enough_distance = (total_meters - last_eeprom_written_total_meters >= EEPROM_UPDATE_EACH_METERS);
  if (traveled_enough_distance || (came_to_stop && millis() - last_eeprom_update_on_stop > EEPROM_UPDATE_MIN_DELAY_ON_STOP)) {
    if (came_to_stop)
      last_eeprom_update_on_stop = millis();
    last_eeprom_written_total_meters = total_meters;
    eeprom_write_volts(data.voltage);
    eeprom_write_mah_spent(mah_spent);
    eeprom_write_total_distance(total_meters);
    eeprom_write_session_data(session_data);
  }

  last_rpm = rpm;

  scr->update(&data);
  scr->heartbeat(UPDATE_DELAY, true);


  button_1_pressed_prev = button_1_pressed;
  button_2_pressed_prev = button_2_pressed;
  button_3_pressed_prev = button_3_pressed;
}
