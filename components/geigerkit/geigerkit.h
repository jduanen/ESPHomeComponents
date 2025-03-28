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

    void set_cpm_sensor(sensor::Sensor *cpm_sensor) { counts_per_min_sensor_ = cpm_sensor; }
    void set_sieverts_sensor(sensor::Sensor *usv_sensor) { uSv_per_hr_sensor_ = usv_sensor; }
    void set_volts_sensor(sensor::Sensor *v_sensor) { volts_sensor_ = v_sensor; }


  protected:
    std::vector<uint8_t> buffer_;

    sensor::Sensor *counts_per_min_sensor_{nullptr};
    sensor::Sensor *uSv_per_hr_sensor_{nullptr};
    sensor::Sensor *volts_sensor_{nullptr};
};

}  // namespace geigerkit_ns
}  // namespace esphome
