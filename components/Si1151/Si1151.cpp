#include "Si1151.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"


namespace esphome {
namespace Si1151 {

static const char *const TAG = "Si1151";

void Si1151Component::setup() {
  uint8_t id;
  ESP_LOGCONFIG(TAG, "Setting up Si1151... 3");

  if (this->read_register(PART_ID, &id, 1) != i2c::ERROR_OK) {
    this->mark_failed();
    ESP_LOGE(TAG, "Si1151 failed setup");
    return;
  }
  ESP_LOGV(TAG, "Si1151 PART_ID: 0x%02x", id);
  if (this->read_register(REV_ID, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGV(TAG, "Si1151 REV_ID: 0x%02x", id);
  }
  if (this->read_register(MFR_ID, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGV(TAG, "Si1151 MFR_ID: 0x%02x", id);
  }
  if (this->read_register(INFO_0, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGV(TAG, "Si1151 INFO_0: 0x%02x", id);
  }
  if (this->read_register(INFO_1, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGV(TAG, "Si1151 INFO_1: 0x%02x", id);
  }

  bool fail = false;
  do {
    if (!send_command_(START)) {
      fail = true;
      break;
    }
    if (!set_parameter_(CHAN_LIST, 0B000010)) {
      fail = true;
      break;
    }
    if (!set_parameter_(MEASRATE_H, 0)) {
      fail = true;
      break;
    }
    // 1 for a base period of 800 us
    if (!set_parameter_(MEASRATE_L, 1)) {
      fail = true;
      break;
    }
    if (!set_parameter_(MEASCOUNT_0, 5)) {
      fail = true;
      break;
    }
    if (!set_parameter_(MEASCOUNT_1, 10)) {
      fail = true;
      break;
    }
    if (!set_parameter_(MEASCOUNT_2, 10)) {
      fail = true;
      break;
    }
    if (!set_parameter_(THRESHOLD0_L, 200)) {
      fail = true;
      break;
    }
    if (!set_parameter_(THRESHOLD0_H, 0)) {
      fail = true;
      break;
    }
  } while (false);
  if (fail) {
    this->mark_failed();
    ESP_LOGE(TAG, "Si1151 failed setup");
    return;
  }

  ESP_LOGV(TAG, "Si1151 successfully set up");

/*
    Wire.beginTransmission(Si115X::DEVICE_ADDRESS);
    Wire.write(Si115X::IRQ_ENABLE);
    Wire.write(0B000010);
    Wire.endTransmission();
*/
};

void Si1151Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Si1151:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with Si1151 failed!");
  }
  LOG_UPDATE_INTERVAL(this);

  LOG_SENSOR("  ", "IR Channel", this->ir_sensor_);
  LOG_SENSOR("  ", "Visible Channel", this->visible_sensor_);
  LOG_SENSOR("  ", "UV Channel", this->uv_sensor_);
};

void Si1151Component::update() {
  ESP_LOGE(TAG, "update():");
  if (ir_sensor_ != nullptr) {
    uint16_t ir = read_IR_();
    ESP_LOGV(TAG, "IR: 0x%02x", ir);
    ir_sensor_->publish_state(ir);
  }
  if (visible_sensor_ != nullptr) {
    uint16_t vis = read_visible_();
    ESP_LOGV(TAG, "VIS: 0x%02x", vis);
    visible_sensor_->publish_state(vis);
  }
  if (uv_sensor_ != nullptr) {
    float uv = read_UV_();
    ESP_LOGV(TAG, "UV: 0x%02x", uv);
    uv_sensor_->publish_state(uv);
  }
};

bool Si1151Component::send_command_(uint8_t code) {
  uint8_t packet[2] = {COMMAND, code};
  uint8_t resp, cmd_ctr;

  do {
    if (this->read_register(RESPONSE_0, &cmd_ctr, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "send_command_: read_register RESPONSE_0 failed");
      return false;
    }
    if (this->write(packet, sizeof(packet)) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "send_command_: write COMMAND data failed");
      return false;
    }
    if (this->read_register(RESPONSE_0, &resp, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "send_command_: read_register RESPONSE_0 failed");
      return false;
    }
  } while (resp > cmd_ctr);
  return true;
};

bool Si1151Component::set_parameter_(uint8_t loc, uint8_t val) {
  uint8_t pkt_0[2] = {HOSTIN_0, val};
  uint8_t pkt_1[2] = {COMMAND, (uint8_t)(loc | (0B10 << 6))};
  uint8_t resp, cmd_ctr;

  do {
    if (this->read_register(RESPONSE_0, &cmd_ctr, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "set_parameter_: read_register RESPONSE_0 failed");
      return false;
    }
    if (this->write(pkt_0, sizeof(pkt_0)) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "set_parameter_: write HOSTIN_0 failed");
      return false;
    }
    if (this->write(pkt_1, sizeof(pkt_1)) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "set_parameter_: write COMMAND failed");
      return false;
    }
    if (this->read_register(RESPONSE_0, &resp, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "set_parameter_: read_register RESPONSE_0 failed");
      return false;
    }
  } while (resp > cmd_ctr);
  return true;
};

int32_t Si1151Component::read_IR_(void) {
  uint8_t data_0, data_1;

  if (!send_command_(FORCE)) {
    ESP_LOGE(TAG, "read_IR_: failed to send FORCE command");
    return -1;
  }
  if (this->read_register(HOSTOUT_1, &data_0, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_IR_: read_register HOSTOUT_1 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV0: 0x%x", data_0);
  if (this->read_register(HOSTOUT_2, &data_1, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_IR_: read_register HOSTOUT_2 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV1: 0x%x", data_1);

  return ((data_1 < 8) + data_0);
};

int32_t Si1151Component::read_visible_(void) {
  uint8_t data_0, data_1;

  if (!send_command_(FORCE)) {
    ESP_LOGE(TAG, "read_visible_: failed to send FORCE command");
    return -1;
  }
  if (this->read_register(HOSTOUT_1, &data_0, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_visible_: read_register HOSTOUT_1 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV0: 0x%x", data_0);
  if (this->read_register(HOSTOUT_2, &data_1, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_visible_: read_register HOSTOUT_2 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV1: 0x%x", data_1);

  return (((data_1 < 8) + data_0) / 3);
};

float Si1151Component::read_UV_(void) {
  uint8_t data_0, data_1;

  if (!send_command_(FORCE)) {
    ESP_LOGE(TAG, "read_UV_: failed to send FORCE command");
    return -1;
  }
  if (this->read_register(HOSTOUT_1, &data_0, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_UV_: read_register HOSTOUT_1 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV0: 0x%x", data_0);
  if (this->read_register(HOSTOUT_2, &data_1, 1) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_UV_: read_register HOSTOUT_2 failed");
    return -1;
  }
  ESP_LOGV(TAG, "UV1: 0x%x", data_1);

  return ((((data_1 < 8) + data_0) / 3) * 0.0012);
};

}  // namespace Si1151
}  // namespace esphome
