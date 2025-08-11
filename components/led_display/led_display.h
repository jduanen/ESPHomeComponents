#pragma once

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/display/display_buffer.h"

#include <numeric>  //// TMP TMP TMP
#include <vector>

/*
#### TODO
####  * add printdigitf(), printdigit(), strftimedigit() methods????
####  * 
*/

namespace esphome {
namespace led_display {


typedef uint8_t LedColor_t;

typedef std::vector<LedColor_t> FrameBufferColumns_t;
typedef std::vector<FrameBufferColumns_t> FrameBuffer_t;

enum ScrollMode {
  CONTINUOUS = 0,
  STOP,
};


static const char *const TAG = "LedDisplay";

static const uint8_t ROW_BIT_0  = D0;
static const uint8_t ROW_BIT_1  = D1;
static const uint8_t ROW_BIT_2  = D2;

static const uint8_t RED_LEDS_ENB   = D3;
static const uint8_t GREEN_LEDS_ENB = D4;

static const uint8_t COL_CLOCK  = D5;
static const uint8_t COL_DATA   = D8;
static const uint8_t COL_STROBE = D9;

static const LedColor_t BLACK_LED_COLOR = 0b00000000;
static const LedColor_t RED_LED_COLOR   = 0b00000001;
static const LedColor_t GREEN_LED_COLOR = 0b00000010;
static const LedColor_t AMBER_LED_COLOR = (GREEN_LED_COLOR | RED_LED_COLOR);

static const uint8_t LED_DISP_HEIGHT = 7;
static const uint8_t LED_DISP_WIDTH = 85;

static const uint MAX_LEDS_ON_DELAY = 2000;  // 2 msec
static const uint MIN_LEDS_ON_DELAY = 1;     // 1 usec


class LedDisplayComponent;

using LedDisplayWriter_t = std::function<void(LedDisplayComponent &)>;

class LedDisplayComponent: public display::DisplayBuffer {
public:
  void set_writer(LedDisplayWriter_t &&writer) { this->writerLocal_ = writer; }
 
  void setup() override;

  void dump_config() override;

  float get_setup_priority() const { return setup_priority::PROCESSOR; };

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_height_internal() { return LED_DISP_HEIGHT; };
  int get_width_internal() { return LED_DISP_WIDTH; };

  display::DisplayType get_display_type() { return display::DisplayType::DISPLAY_TYPE_COLOR; }

  void update() override;

  void loop() override;

  void set_intensity(uint8_t intensity) {
    this->intensity_ = intensity;
    this->brightness_ = map(intensity, 0, 100, MIN_LEDS_ON_DELAY, MAX_LEDS_ON_DELAY);
    ESP_LOGV(TAG, "Set Intensity: %u\nBrightness: %u", this->intensity_, this->brightness_);
  }
  void set_scroll_mode(ScrollMode mode) {
    this->scrollMode_ = mode;
    ESP_LOGV(TAG, "ScrollMode: %u", mode);
  };
  void set_scroll(bool onOff) {
    this->scrollingOn_ = onOff;
    ESP_LOGV(TAG, "Scroll: %u", onOff);
  };
  void set_scroll_speed(uint16_t speed) { this->scrollSpeed_ = speed; };
  void set_scroll_dwell(uint16_t dwell) { this->scrollDwell_ = dwell; };
  void set_scroll_delay(uint16_t delay) { this->scrollDelay_ = delay; };

  void turn_on_off(bool onOff) { this->displayOn_ = onOff; };

  void invert_on_off(bool onOff) { this->invert_ = onOff; };
  void invert_on_off() { this->invert_ = !this->invert_; };

  void intensity(uint8_t intensity) {
    this->intensity_ = intensity;
    this->brightness_ = map(intensity, 0, 100, MIN_LEDS_ON_DELAY, MAX_LEDS_ON_DELAY);
    ESP_LOGV(TAG, "Intensity: %u\nBrightness: %u", this->intensity_, this->brightness_);
  };

protected:
  FrameBuffer_t frameBuffer_;

  LedColor_t background_ = BLACK_LED_COLOR;

  bool update_ = false;

  uint16_t oldBufferWidth_;

  bool displayOn_ = false;

  bool invert_ = false;

  uint8_t intensity_;  //// Intensity of the display from 0 to 100 (brightest)
  uint brightness_;    //// Intensity mapped into num of uSec linger time

  bool scrollingOn_;
  uint32_t lastScroll_;
  uint32_t lastLoop_;
  uint16_t stepsLeft_;
  uint16_t scrollDwell_;
  uint16_t scrollDelay_;
  uint16_t scrollSpeed_;
  ScrollMode scrollMode_;

  optional<LedDisplayWriter_t> writerLocal_{};

  void scrollLeft_();

  void scroll_(bool onOff, ScrollMode mode, uint16_t speed, uint16_t delay, uint16_t dwell) {
    this->set_scroll(onOff);
    this->set_scroll_mode(mode);
    this->set_scroll_speed(speed);
    this->set_scroll_dwell(dwell);
    this->set_scroll_delay(delay);
  };

  void scroll_(bool onOff, ScrollMode mode) {
    this->set_scroll(onOff);
    this->set_scroll_mode(mode);
  };

  void scroll_(bool onOff) {
    this->set_scroll(onOff);
  };

  LedColor_t colorToLedColor(Color color);

  void display_();

  void enableRow_(LedColor_t rowColor, uint rowNum);
  void disableRows_();
  void shiftInPixels_(LedColor_t rowColor, uint rowNum);
};

}  // namespace led_display
}  // namespace esphome
