#include "geigerkit.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace geigerkit_ns {

static const char *TAG = "GeigerKit";

void GeigerKitSensor::setup() {
  ESP_LOGD(TAG, "Setting up GeigerKit");
  // Rx-only, no setup needed
  /*
  // Clear UART buffer
  while (this->available())
    this->read();
  */
  if (false) mark_failed();
}

void GeigerKitSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "GeigerKit:");
  this->check_uart_settings(9600);
}

void GeigerKitSensor::loop() {
  uint8_t data;

  const uint32_t now = millis();
  if ((now - this->last_read_) > 500 && this->available()) {
    ESP_LOGD(TAG, "CHECK");
    while (this->available())
      this->read_byte(&data);
      ESP_LOGD(TAG, "READ: 0x%x", data);
    this->last_read_ = now;
  }

/*
  while (this->available() != 0) {
    ESP_LOGD(TAG, "DA");
    this->read_byte(&data);
    if (this->buffer_.empty() && (data != 0xff))
      continue;
    ESP_LOGD(TAG, "Data: 0x%x", data);
    this->buffer_.push_back(data);
    if (this->buffer_.size() == 4)
      this->check_buffer_();
  }
*/
}

void GeigerKitSensor::check_buffer_() {
  ESP_LOGD(TAG, "Data read from sensor: %s", format_hex_pretty(this->buffer_).c_str());
  this->buffer_.clear();
}


}  // namespace geigerkit_ns
}  // namespace esphome
