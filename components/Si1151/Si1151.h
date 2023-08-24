#pragma once

#include "esphome.h"
#include <Si115X.h>


class Si1151 : public PollingComponent, public Sensor {
  public:
    Si115X sls;
    Sensor *ir_sensor = new Sensor();
    Sensor *vis_sensor = new Sensor();
    Sensor *uv_sensor = new Sensor();

    Si1151() : PollingComponent(60000) {};

    float get_setup_priority() const override {
      return setup_priority::DATA;
    };

    void setup() override { sls.Begin(); };

    void update() override {
      uint16_t ir = sls.ReadHalfWord();
      ir_sensor->publish_state(ir);
      uint16_t vis = sls.ReadHalfWord_VISIBLE();
      vis_sensor->publish_state(vis);
      float uv = sls.ReadHalfWord_UV();
      uv_sensor->publish_state(uv);
    };
    /*
    void setup() override {};
    void update() override { publish_state(42.0); };
    */
};
