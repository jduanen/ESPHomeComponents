#include "geigerkit.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace geigerkit_sensor_ns {

static const char *TAG = "geigerkit";

geigerkit::geigerkit() {
  ESP_LOGD(TAG, "Constructor");
}

geigerkit::geigerkit(const std::string &custom_id) {
  ESP_LOGD(TAG, "Constructor");
  set_custom_id(custom_id);
}

void geigerkit::setup() {
  ESP_LOGD(TAG, "Setting up geigerkit with custom_id: %s", this->custom_id_.c_str());
  //// TODO poke the device and see if it responds as expected
  if (false) mark_failed();
}

void geigerkit::dump_config() {
  ESP_LOGCONFIG(TAG, "geigerkit:");
  ESP_LOGCONFIG(TAG, "  Custom ID: %s", this->custom_id_.c_str());
}

/*
void geigerkit::loop() {
  uint8_t data;
  while (this->available() > 0) {
    this->read_byte(&data);
    if (this->buffer_.empty() && (data != 0xff))
      continue;
    buffer_.push_back(data);
    if (this->buffer_.size() == 4)
      this->check_buffer_();
  }
}
*/

void geigerkit::check_buffer_() {
  ESP_LOGW(TAG, "Data read from sensor: %s", format_hex_pretty(this->buffer_).c_str());
  this->buffer_.clear();
}


}  // namespace geigerkit_sensor_ns
}  // namespace esphome
