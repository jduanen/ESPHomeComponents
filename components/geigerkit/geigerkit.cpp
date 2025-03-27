#include "geigerkit.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace geigerkit_ns {

static const char *TAG = "GeigerKit";
GeigerKitComponent::GeigerKitComponent() {
  ESP_LOGD(TAG, "Constructing");
}

void GeigerKitComponent::setup() {
  ESP_LOGD(TAG, "Setting up GeigerKit");
  // Rx-only, no setup needed
  //// TODO poke the device and see if it responds as expected
  if (false) mark_failed();
}

void GeigerKitComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "GeigerKit:");
//  this->check_uart_settings(9600);
}

void GeigerKitComponent::loop() {
  ESP_LOGD(TAG, "Loop");

  /*
  uint8_t data;

  while (this->available() != 0) {
    this->read_byte(&data);
    if (this->buffer_.empty() && (data != 0xff))
      continue;
    this->buffer_.push_back(data);
    if (this->buffer_.size() == 4)
      this->check_buffer_();
  }
  */
}

void GeigerKitComponent::check_buffer_() {
  ESP_LOGD(TAG, "Data read from sensor: %s", format_hex_pretty(this->buffer_).c_str());
//  this->buffer_.clear();
}


}  // namespace geigerkit_ns
}  // namespace esphome
