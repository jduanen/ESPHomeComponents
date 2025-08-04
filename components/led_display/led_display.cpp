#include "esphome.h"
#include "led_display.h"


namespace esphome {
namespace led_display {

static const char *const TAG = "LedDisplay";

//// TODO put in the interface pins

void LedDisplayComponent::setup() {
  ESP_LOGV(TAG, "SETUP");
}

void LedDisplayComponent::dump_config() {
  ESP_LOGCONFIG(TAG,
                "LED_DISPLAY:\n"
                "  Brightness: %u\n",
                this->brightness_);
//  LOG_PIN("  CS Pin: ", this->cs_);
}

void LedDisplayComponent::draw_absolute_pixel_internal(int x, int y, Color color) {
  ESP_LOGV(TAG, "Draw Abs: x=%d, y=%d, c=0x%x", x, y, color);
}

void LedDisplayComponent::update() {
  ESP_LOGV(TAG, "Update");
}

void LedDisplayComponent::loop() {
  //ESP_LOGV(TAG, "Loop");
}

}  // namespace led_display
}  // namespace esphome

/*
class LedDisplayComponent : public esphome::display::DisplayBuffer {
 public:
  //// TODO consider a constructor with starting brightness setting, otherwise default
  LedDisplayComponent() : esphome::display::DisplayBuffer() {}
  LedDisplayComponent() : esphome::display::DisplayBuffer() {}

  void setup() override {
    //// TODO: Initialize display hardware
    ESP_LOGCONFIG("led_display", "LedDisplay setup");
  }

  void dump_config() override {
    ESP_LOGCONFIG("led_display", "LedDisplay configuration:");
    //// TODO: (optional) print config details like dimensions, etc.
  }

  // This method writes a pixel color into the internal framebuffer at (x,y)
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override {
    // cast buffer_ to 16-bit color array (RGB565 assumed)
    uint16_t *buf16 = (uint16_t*)this->buffer_;
    int index = y * this->get_width() + x;
    buf16[index] = color.to_rgb565();
  }

  // flush the framebuffer to the actual hardware
  void update() override {
    //// FIXME
    /
    // For example, clear the buffer with black
    this->fill_(esphome::color::BLACK);
    // Draw some text at (0,0) using a font defined in YAML as 'my_font'
    this->print(0, 0, id(my_font), esphome::color::WHITE, "Hello my custom display!");
    // Send framebuffer to actual hardware display
    /
    this->flush();
  }
};
*/
