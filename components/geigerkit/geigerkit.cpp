#include "geigerkit.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace geigerkit_ns {

static const char *TAG = "GeigerKit";

void GeigerKitSensor::setup() {
  ESP_LOGD(TAG, "Setting up GeigerKit");
  // Rx-only, no setup needed
  this->flush();
  ////if (false) mark_failed();
}

void GeigerKitSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "GeigerKit:");
  this->check_uart_settings(9600);
}

void GeigerKitSensor::loop() {
  uint8_t data;

  while (this->available() != 0) {
    this->read_byte(&data);
    if (data == '\n') continue;
    if (data == '\r') {
      int countsPerMin;
      float uSvPerHr, volts;
      char tmpBuf[32];
      ESP_LOGD(TAG, "Str: %s", this->buffer_.data());
      memcpy(tmpBuf, this->buffer_.data(), this->buffer_.size());
      int n = sscanf(tmpBuf, "%d,%f,%f", &countsPerMin, &uSvPerHr, &volts);
      if (n != 3) {
          ESP_LOGE("custom", "Failed to read from GK sensor board: %s", this->buffer_.data());
      }
      ESP_LOGD(TAG, "cpm: %d, Sv/Hr: %f, volts: %f");
      /*
      this->sensor->publish_state(countsPerMin);
      this->sensor->publish_state(uSvPerHr);
      this->sensor->publish_state(volts);
      */
      this->buffer_.clear();
      continue;
    }
    this->buffer_.push_back(data);
  }
}

/*
    if (this->buffer_.size() == 4)
      this->check_buffer_();
*/

void GeigerKitSensor::check_buffer_() {
  ESP_LOGD(TAG, "Data read from sensor: %s", format_hex_pretty(this->buffer_).c_str());
  this->buffer_.clear();
}


}  // namespace geigerkit_ns
}  // namespace esphome
