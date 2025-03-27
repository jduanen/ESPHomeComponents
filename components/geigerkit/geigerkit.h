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
};

}  // namespace geigerkit_ns
}  // namespace esphome
