#include "Si1151.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"


namespace esphome {
namespace Si1151 {

static const char *const TAG = "Si1151";

void Si1151Component::setup() {
  uint8_t id;
  ESP_LOGCONFIG(TAG, "Setting up Si1151...1");

  if (this->read_register(PART_ID, &id, 1) != i2c::ERROR_OK) {
    this->mark_failed();
    ESP_LOGE(TAG, "Si1151 failed setup");
    return;
  }

  send_command(START);
  ESP_LOGV(TAG, "Si1151 successfully set up");

/*
    Si115X::param_set(Si115X::CHAN_LIST, 0B000010);

    Si115X::param_set(Si115X::MEASRATE_H, 0);
    Si115X::param_set(Si115X::MEASRATE_L, 1);  // 1 for a base period of 800 us
    Si115X::param_set(Si115X::MEASCOUNT_0, 5); 
    Si115X::param_set(Si115X::MEASCOUNT_1, 10);
    Si115X::param_set(Si115X::MEASCOUNT_2, 10);
    Si115X::param_set(Si115X::THRESHOLD0_L, 200);
    Si115X::param_set(Si115X::THRESHOLD0_H, 0);

    Wire.beginTransmission(Si115X::DEVICE_ADDRESS);
    Wire.write(Si115X::IRQ_ENABLE);
    Wire.write(0B000010);
    Wire.endTransmission();

  ?;
  ?;
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
  ESP_LOGE(TAG, "update(): TBD");
/*
  if (this->read_data_register_(TCS34725_REGISTER_CDATAL, raw_c) != i2c::ERROR_OK) {
    this->status_set_warning();
    return;
  }
  ESP_LOGV(TAG, "Raw values clear=%d red=%d green=%d blue=%d", raw_c, raw_r, raw_g, raw_b);

  uint16_t ir = sls.ReadHalfWord();
  ir_sensor->publish_state(ir);
  uint16_t vis = sls.ReadHalfWord_VISIBLE();
  vis_sensor->publish_state(vis);
  float uv = sls.ReadHalfWord_UV();
  uv_sensor->publish_state(uv);

  ?;
*/
};

/*
int read_register(uint8_t reg, int bytes) {
  int val;

  write_data(&reg, sizeof(reg));
  request(bytes);
  if (available) {
    val = read();
  } else {
    val = -1
  }
  return val;
}
*/

void send_command(uint8_t code) {
  uint8_t packet[2];
  int resp, cmd_ctr;

  packet[0] = COMMAND;
  packet[1] = code;
  do {
    if (this->read_register(RESPONSE_0, &cmd_ctr, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "send_command: read_register failed");
      continue;
    }
    write_data(packet, sizeof(packet));
    if (this->read_register(RESPONSE_0, &resp, 1) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "send_command: read_register failed");
      continue;
    }
  } while (resp > cmd_ctr);
};


}  // namespace Si1151
}  // namespace esphome
