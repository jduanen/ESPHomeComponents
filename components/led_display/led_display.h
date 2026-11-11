#pragma once

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_cpu.h"
#include "soc/gpio_reg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/font/font.h"
#include "esphome/components/display/display_buffer.h"

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

// The display is refreshed from a hardware timer interrupt, one (row, color)
// "phase" at a time: 7 rows x 2 colors = 14 phases per frame.
// 14 x 600us => ~119Hz frame rate, independent of what the main loop is doing.
static const uint8_t NUM_SCAN_PHASES = (LED_DISP_HEIGHT * 2);
static const uint32_t SCAN_PHASE_US = 600;
static const uint32_t SHIFT_TIME_US = 80;      // budget for shifting in one row (85 bits, ~55us)
static const uint32_t MIN_LEDS_OFF_US = 30;    // always leave the CPU some idle time per phase
static const uint MAX_LEDS_ON_DELAY = (SCAN_PHASE_US - SHIFT_TIME_US - MIN_LEDS_OFF_US);
static const uint MIN_LEDS_ON_DELAY = 1;       // 1 usec

// MC14094 timing (worst case @5V): clock high >= 200ns, data setup >= 125ns,
// strobe >= 200ns.  Expressed in CPU cycles @160MHz, with margin.
static const uint32_t SETUP_CYCLES = 32;       // 200ns
static const uint32_t PULSE_CYCLES = 48;       // 300ns

static const uint32_t ROW_BITS_MASK = ((1UL << ROW_BIT_0) | (1UL << ROW_BIT_1) | (1UL << ROW_BIT_2));
static const uint32_t ENABLES_MASK = ((1UL << RED_LEDS_ENB) | (1UL << GREEN_LEDS_ENB));

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

  void invert_on_off(bool onOff) { this->invert_ = onOff; this->dirty_ = true; };
  void invert_on_off() { this->invert_ = !this->invert_; this->dirty_ = true; };

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

  bool flipX_;

  volatile bool displayOn_ = false;

  bool invert_ = false;

  uint8_t intensity_;           //// Intensity of the display from 0 to 100 (brightest)
  volatile uint brightness_;    //// Intensity mapped into num of uSec LEDs are on per phase

  bool scrollingOn_;
  uint32_t lastScroll_;
  uint16_t scrollOffset_;       //// current horizontal scroll position (in columns)
  uint16_t scrollDwell_;
  uint16_t scrollDelay_;
  uint16_t scrollSpeed_;
  ScrollMode scrollMode_;

  std::vector<Font *> fonts_;
  uint8_t numFonts_ = 0;

  Font *currentFont_;
  LedColor_t currentColor_;

  optional<LedDisplayWriter_t> writerLocal_{};

  // Double-buffered scan data, already in shift-out order (one byte per
  // column bit) for each (physical row, color) phase.  The main loop writes
  // the back buffer and requests a swap; the ISR swaps at the start of a frame.
  uint8_t scanBuf_[2][NUM_SCAN_PHASES][LED_DISP_WIDTH];
  volatile uint8_t shownBuf_ = 0;
  volatile bool swapPending_ = false;
  bool dirty_ = false;          //// visible window needs to be re-composed

  gptimer_handle_t timer_ = nullptr;
  uint8_t phase_ = 0;
  bool ledsOn_ = false;

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

  void composeScanBuffer_();

  static bool onTimerAlarm_(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *arg);
  uint32_t refreshPhase_();

  long map_(long x, long inMin, long inMax, long outMin, long outMax) {
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  };
};

}  // namespace led_display
}  // namespace esphome
