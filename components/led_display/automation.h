#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"

#include "led_display.h"


namespace esphome {
namespace led_display {

template<typename... Ts> class DisplayInvertAction : public Action<Ts...>, public Parented<LedDisplayComponent> {
 public:
  TEMPLATABLE_VALUE(bool, state)

  void play(Ts... x) override {
    bool state = this->state_.value(x...);
    this->parent_->invert_on_off(state);
  }
};

template<typename... Ts> class DisplayVisibilityAction : public Action<Ts...>, public Parented<LedDisplayComponent> {
 public:
  TEMPLATABLE_VALUE(bool, state)

  void play(Ts... x) override {
    bool state = this->state_.value(x...);
    this->parent_->turn_on_off(state);
  }
};

template<typename... Ts> class DisplayReverseAction : public Action<Ts...>, public Parented<LedDisplayComponent> {
 public:
  TEMPLATABLE_VALUE(bool, state)

  void play(Ts... x) override {
    bool state = this->state_.value(x...);
    this->parent_->set_reverse(state);
  }
};

template<typename... Ts> class DisplayIntensityAction : public Action<Ts...>, public Parented<LedDisplayComponent> {
 public:
  TEMPLATABLE_VALUE(uint8_t, state)

  void play(Ts... x) override {
    uint8_t state = this->state_.value(x...);
    this->parent_->set_intensity(state);
  }
};

}  // namespace led_display
}  // namespace esphome
