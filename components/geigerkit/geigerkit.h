#pragma once

#include "esphome/core/component.h"
//#include "esphome/components/sensor/sensor.h"
//#include "esphome/components/uart/uart.h"

namespace esphome {
namespace geigerkit_sensor_ns {

//class GeigerKitSensor : public sensor::Sensor, public Component, public uart::UARTDevice {
class geigerkit : public Component {
 public:
  geigerkit();
  geigerkit(const std::string &custom_id);

  void set_custom_id(const std::string &custom_id) { custom_id_ = custom_id; }

  void setup() override;
  void dump_config() override;
//  void loop() override;

 protected:
  std::string custom_id_ = "NO_ID";
  void check_buffer_();

  std::vector<uint8_t> buffer_;
};

}  // namespace geigerkit_sensor_ns
}  // namespace esphome
