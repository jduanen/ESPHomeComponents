#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
//#include "esphome/components/sensor/sensor.h"
//#include "esphome/components/uart/uart.h"

namespace esphome {
namespace geigerkit_ns {

class GeigerKitComponent: public Component, public uart::UARTDevice {
  public:
    GeigerKitComponent() = default;
//    GeigerKitComponent();

    void setup() override;
    void dump_config() override;
    void loop() override;
    float get_setup_priority() const { return setup_priority::DATA; };

  protected:
    void check_buffer_();

    std::vector<uint8_t> buffer_;
};

}  // namespace geigerkit_ns
}  // namespace esphome
