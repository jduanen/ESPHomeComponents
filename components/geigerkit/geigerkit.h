#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace geigerkit_ns {

class GeigerKitSensor: public Component, public uart::UARTDevice {
  public:
    GeigerKitSensor() = default;

    void setup() override;
    void dump_config() override;
    void loop() override;
    float get_setup_priority() const { return setup_priority::DATA; };

  protected:
//    uint32_t last_read_ = 0;
    std::vector<uint8_t> buffer_;

  sensor::Sensor *counts_per_min_sensor_ = new sensor::Sensor();
  sensor::Sensor *uSv_per_hr_sensor_ = new sensor::Sensor();
  sensor::Sensor *volts_sensor_ = new sensor::Sensor();
};

}  // namespace geigerkit_ns
}  // namespace esphome
