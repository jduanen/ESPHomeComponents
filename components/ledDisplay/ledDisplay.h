#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace leddisplay {

static const uint8_t LED_DISP_HEIGHT = 7;
static const uint8_t LED_DISP_WIDTH = 85;

class LedDisplayComponent: public display::DisplayBuffer {
public:
	void setup() override;

	void dump_config() override;

	float get_setup_priority() const { return setup_priority::PROCESSOR; };

  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_height_internal() { return LED_DISP_HEIGHT; };
  int get_width_internal() { return LED_DISP_WIDTH; };

  void set_brightness(uint8_t brightness) { this->brightness_ = brightness; };

  display::DisplayType get_display_type() { return display::DisplayType::DISPLAY_TYPE_COLOR; }

	void update() override;

	void loop() override;

protected:
	uint8_t brightness_;	// brightness of display from 0-100%
};


}  // namespace leddisplay
}  // namespace esphome
