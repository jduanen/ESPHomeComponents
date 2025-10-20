#pragma once

#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/font/font.h"
#include "esphome/components/display/display_buffer"

#include <numeric>
#include <vector>

/*
#### TODO
####  * add printdigitf(), printdigit(), strftimedigit() methods????
####  * 
*/

namespace esphome {
namespace led_display {


using esphome::font::Font;

typedef uint8_t LedColor_t;

typedef std::vector<LedColor_t> FrameBufferColumns_t;
typedef std::vector<FrameBufferColumns_t> FrameBuffer_t;

enum ScrollMode {
  CONTINUOUS = 0,
  STOP,
};


static const char *const TAG = "LedDisplay";

static const gpio_num_t ROW_BIT_0  = GPIO_NUM_2;  // D0
static const gpio_num_t ROW_BIT_1  = GPIO_NUM_3;  // D1
static const gpio_num_t ROW_BIT_2  = GPIO_NUM_4;  // D2

static const gpio_num_t RED_LEDS_ENB   = GPIO_NUM_5;  // D3
static const gpio_num_t GREEN_LEDS_ENB = GPIO_NUM_6;  // D4

static const gpio_num_t COL_CLOCK  = GPIO_NUM_7;  // D5
static const gpio_num_t COL_DATA   = GPIO_NUM_8;  // D8
static const gpio_num_t COL_STROBE = GPIO_NUM_9;  // D9

static const LedColor_t BLACK_LED_COLOR = 0b00000000;
static const LedColor_t RED_LED_COLOR   = 0b00000001;
static const LedColor_t GREEN_LED_COLOR = 0b00000010;
static const LedColor_t AMBER_LED_COLOR = (GREEN_LED_COLOR | RED_LED_COLOR);
static const uint8_t MAX_NUM_COLORS = 4;

static const uint8_t LED_CHAR_HEIGHT = 7;
static const uint8_t LED_CHAR_WIDTH = 5;

static const uint8_t LED_NUM_CHARS = 17;

static const uint8_t LED_DISP_HEIGHT = LED_CHAR_HEIGHT;
static const uint8_t LED_DISP_WIDTH = (LED_CHAR_WIDTH * LED_NUM_CHARS);

static const uint MAX_LEDS_ON_DELAY = 2000;  // 2 msec
static const uint MIN_LEDS_ON_DELAY = 1;     // 1 usec

static const LedColor_t COLORS[] = {
  BLACK_LED_COLOR,
  RED_LED_COLOR,
  GREEN_LED_COLOR,
  AMBER_LED_COLOR
};


class LedDisplayComponent;

using LedDisplayWriter_t = std::function<void(LedDisplayComponent &)>;

class LedDisplayComponent: public display::DisplayBuffer {
public:
  void set_fonts(const std::vector<Font *> fonts) {
    this->fonts_ = fonts;
    this->numFonts_ = fonts.size();
  }

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
    this->brightness_ = map_(intensity, 0, 100, MIN_LEDS_ON_DELAY, MAX_LEDS_ON_DELAY);
    ESP_LOGV(TAG, "Set Intensity: %u\nBrightness: %u", this->intensity_, this->brightness_);
  };

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

  void set_flip_x(bool flipX) { this->flipX_ = flipX; };

  void turn_on_off(bool onOff) { this->displayOn_ = onOff; };

  void invert_on_off(bool onOff) { this->invert_ = onOff; };
  void invert_on_off() { this->invert_ = !this->invert_; };

  void intensity(uint8_t intensity) {
    this->intensity_ = intensity;
    this->brightness_ = map_(intensity, 0, 100, MIN_LEDS_ON_DELAY, MAX_LEDS_ON_DELAY);
    ESP_LOGV(TAG, "Intensity: %u\nBrightness: %u", this->intensity_, this->brightness_);
  };

  void clear();

  //// TODO add printfLED() methods that take print format strings and args

  uint8_t printLED(const char *str) { return this->printLED(0, str); };
  uint8_t printLED(uint8_t startPos, const char *str);

protected:
  FrameBuffer_t frameBuffer_;

  LedColor_t background_ = BLACK_LED_COLOR;

  bool update_ = false;

  uint16_t oldBufferWidth_;

  bool flipX_;

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

  std::vector<Font *> fonts_;
  uint8_t numFonts_ = 0;

  Font *currentFont_;
  LedColor_t currentColor_;

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
  Color LedColorToColor(LedColor_t color);

  uint32_t getStringWidth_(Font *font, const char *str);

  void display_();

  void enableRow_(LedColor_t rowColor, uint rowNum);
  void disableRows_();
  void shiftInPixels_(LedColor_t rowColor, uint rowNum);

  long map_(long x, long inMin, long inMax, long outMin, long outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  };
};

}  // namespace led_display
}  // namespace esphome
