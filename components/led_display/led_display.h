#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display_buffer.h"

#include <vector>


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

static const uint8_t COL_DATA   = D8;
static const uint8_t COL_STROBE = D9;
static const uint8_t COL_CLOCK  = D5;

static const LedColor_t BLACK_LED_COLOR = 0b00000000;
static const LedColor_t RED_LED_COLOR   = 0b00000001;
static const LedColor_t GREEN_LED_COLOR = 0b00000010;
static const LedColor_t AMBER_LED_COLOR = (GREEN_LED_COLOR | RED_LED_COLOR);

static const uint8_t LED_DISP_HEIGHT = 7;
static const uint8_t LED_DISP_WIDTH = 85;


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

protected:
  FrameBuffer_t frameBuffer_;

  bool update_ = false;

  uint16_t oldBufferWidth_;

  bool displayOn_;

  bool scroll_;
  uint32_t lastScroll_;
  uint16_t stepsLeft_;
  uint16_t scrollSwell_;
  uint16_t scrollDelay_;
  uint16_t scrollSpeed_;
  ScrollMode scrollMode_;

  optional<LedDisplayWriter_t> writerLocal_{};

  /*//// FIXME vvvvvvvvvvvvvvvvvv
  uint8_t intensity_;  //// Intensity of the display from 0 to ? (brightest)
  bool reverse_;
  bool flip_x_;
  bool invert_ = false;
  uint8_t orientation_;
  uint8_t bckgrnd_ = 0x0;
  size_t get_buffer_length_();
  *//// FIXME ^^^^^^^^^^^^^^^^^

  void turnOnOff_(bool onOff);

  void scrollLeft_();
  void setScrollSpeed_(uint16_t speed) { this->scrollSpeed_ = speed; };
  void setScrollDwell_(uint16_t dwell) { this->scrollDwell_ = dwell; };
  void setSrollDelay_(uint16_t delay) { this->scrollDelay_ = delay; };
  void setScroll_(bool onOff) { this->scroll_ = onOff; };
  void setScrollMode_(ScrollMode mode) { this->scrollMode_ = mode; };
  void scroll_(bool onOff, ScrollMode mode, uint16_t speed, uint16_t delay, uint16_t dwell);
  void scroll_(bool onOff, ScrollMode mode);
  void scroll_(bool onOff);

	LedColor_t colorToLedColor(Color color);

	void scrollLeft_();
  void display_();

	void enableRow_(int ledColor, int rowNum);
	void disableRows_();
  void shiftInPixels_(int row, int ledColor);
};

}  // namespace led_display
}  // namespace esphome
