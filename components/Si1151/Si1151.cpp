// Datasheet: https://www.silabs.com/documents/public/data-sheets/si115x-datasheet.pdf
// User's Guide: https://www.silabs.com/documents/public/application-notes/AN950-Si1153-UG.pdf

#include "Si1151.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"


namespace esphome {
namespace Si1151 {

static const char *const TAG = "Si1151";


void Si1151Component::set_ir_sensor(sensor::Sensor *ir_sensor) {
  ir_sensor_ = ir_sensor;
  enabled_channels_.set(IR_CHANNEL);
};

void Si1151Component::set_visible_sensor(sensor::Sensor *visible_sensor) {
  visible_sensor_ = visible_sensor;
  enabled_channels_.set(VISIBLE_CHANNEL);
};

void Si1151Component::set_uv_sensor(sensor::Sensor *uv_sensor) {
  uv_sensor_ = uv_sensor;
  enabled_channels_.set(UV_CHANNEL);
};

void Si1151Component::setup() {
  uint8_t id;
  ESP_LOGCONFIG(TAG, "Setting up Si1151... 3");

  num_channels_ = ((enabled_channels_.test(IR_CHANNEL) ? 1 : 0) +
	 	   (enabled_channels_.test(VISIBLE_CHANNEL) ? 1 : 0) +
                   (enabled_channels_.test(UV_CHANNEL) ? 1 : 0));

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
    ESP_LOGVV(TAG, "Si1151 MFR_ID: 0x%02x", id);
  }
  if (this->read_register(INFO_0, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGVV(TAG, "Si1151 INFO_0: 0x%02x", id);
  }
  if (this->read_register(INFO_1, &id, 1) == i2c::ERROR_OK) {
    ESP_LOGVV(TAG, "Si1151 INFO_1: 0x%02x", id);
  }
  ESP_LOGV(TAG, "Si1151 Enabled Channels: IR=%s, VIS=%s, UV=%s",
           (enabled_channels_.test(IR_CHANNEL) ? "true" : "false"),
           (enabled_channels_.test(VISIBLE_CHANNEL) ? "true" : "false"),
           (enabled_channels_.test(UV_CHANNEL) ? "true" : "false"));

  reset_();
  ESP_LOGV(TAG, "Si1151 successfully set up");
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
  if (enabled_channels_.test(IR_CHANNEL)) {
    uint16_t ir = read_IR_();
    ESP_LOGV(TAG, "IR: 0x%02x", ir);
    ir_sensor_->publish_state(ir);
  }
  if (enabled_channels_.test(VISIBLE_CHANNEL)) {
    uint16_t vis = read_visible_();
    ESP_LOGV(TAG, "VIS: 0x%02x", vis);
    visible_sensor_->publish_state(vis);
  }
  if (enabled_channels_.test(UV_CHANNEL)) {
     float uv = read_UV_();
    ESP_LOGV(TAG, "UV: 0x%02x", uv);
    uv_sensor_->publish_state(uv);
  }
};

i2c::ErrorCode read_register_(uint8_t reg, uint8_t &data) {
  // return values:
  //   0: read successful
  // !=0: RESPONSE_0 error code
  uint8_t buffer;

  i2c::ErrorCode err_code = this->read_register(reg, &buffer, 1);
  if (err_code & RSP0_CMD_ERR) {
    ESP_LOGE(TAG, "read_register_() failure: 0x%02x", err_code);
    return err_code;
  }
  data = buffer;
  return 0;
};

i2c::ErrorCode write_register_(uint8_t reg, uint8_t data) {
  // return values:
  //   0: read successful
  // !=0: RESPONSE_0 error code
  i2c::ErrorCode err_code = this->write_register(reg, &data, 1);
  if (err_code & RSP0_CMD_ERR) {
    ESP_LOGE(TAG, "write_register_() failure: 0x%02x", err_code);
    return err_code;
  }
  return 0;
};

int16_t Si1151Component::wait_until_ready_(void) {
  // return values:
  //  <0: error in reading register
  //   0: device is ready
  //  >0: RESPONSE_0 error code
  // 
  //  RESPONSE0 will go to "RUNNING" after reset and then "SLEEP" after 
  // initialization is complete.
  int const MAX_RETRIES = 3;
  uint8_t response0;
  uint8_t count = 0;
  i2c::ErrorCode err_code;

  while (count < MAX_RETRIES) {
    err_code = read_register_(RESPONSE_0, &response0);
    if (err_code & RSP0_CMD_ERR) {
      return err_code;
    }
    if (response0 & RSP0_SLEEP) {
      break;
    }
    count++;
  }
  return 0;
};

int16_t Si1151Component::hard_reset_(void) {
  // return values:
  //  !0: failure to initialize device
  //  =0: successful reset  delay(30);  // don't access device less than 25ms after POR
  if ((int16_t resval = send_command_(RESET_SW))) {
    return retval;
  }
  return reset_();
};

int16_t Si1151Component::reset_(void) {
  // return values:
  //  !0: failure to initialize device
  //  =0: successful reset
  uint8_t chans = (int)(enabled_channels_.to_ulong());
  int16_t resval;

  // wait until device is in SLEEP mode
  if ((resval = wait_until_ready_())) {
    return resval;
  }

  // init global parameters
  if ((resval = set_parameter_(CHAN_LIST, chans))) {
    return resval;
  }
  ESP_LOGVV(TAG, "Enabled Channels: 0x%02x", chans);

  // only going to use Force mode, so disable everything
  uint8_t failures = set_parameter_(MEASRATE_H, 0);  // 0 * 800usec -- not used
  failures += set_parameter_(MEASRATE_L, 0);  // ...

  failures += set_parameter_(MEASCOUNT_0, 0);  // not used
  failures += set_parameter_(MEASCOUNT_1, 0);  // ...
  failures += set_parameter_(MEASCOUNT_2, 0);

  failures += set_parameter_(THRESHOLD0_L, 0);  // not used
  failures += set_parameter_(THRESHOLD0_H, 0);  // ...
  failures += set_parameter_(THRESHOLD1_L, 0);
  failures += set_parameter_(THRESHOLD1_H, 0);

  failures += set_parameter_(BURST, 0);  // not used

  failures += set_parameter_(U_THRESHOLD_L, 0);  // not used
  failures += set_parameter_(U_THRESHOLD_H, 0);  // ...
  failures += set_parameter_(L_THRESHOLD_L, 0);
  failures += set_parameter_(L_THRESHOLD_H, 0);
  if (failures) {
    ESP_LOGE(TAG, "Failed to initialize");
    return -1;
  }

  //// TODO set up the params for the three channels
  //// FIXME FIXME FIXME

/*
    retval += Si115xParamSet( si115x_handle, PARAM_CH_LIST, 0x0f);

    retval += Si115xParamSet( si115x_handle, PARAM_ADCCONFIG0, 0x78);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCSENS0, 0x09);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCPOST0, 0x40);

    retval += Si115xParamSet( si115x_handle, PARAM_ADCCONFIG1, 0x4d);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCSENS1, 0x61);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCPOST1, 0x40);

    retval += Si115xParamSet( si115x_handle, PARAM_ADCCONFIG2, 0x41);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCSENS2, 0x61);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCPOST2, 0x50);

    retval += Si115xParamSet( si115x_handle, PARAM_ADCCONFIG3, 0x4d);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCSENS3, 0x07);
    retval += Si115xParamSet( si115x_handle, PARAM_ADCPOST3, 0x40);
*/
  return write_register_(IRQ_ENABLE, &chans);
};

uint16_t Si1151Component::get_cmd_ctr_(void) {
  // return results:
  // !0: failed to get cmd_ctr
  // =0: successful got cmd_ctr
  int const MAX_RETRIES = 5;
  uint8_t count = 0;
  i2c::ErrorCode err_code;
  uint8_t cmd_ctr;
  uint8_t response0;

  if ((err_code = read_register_(RESPONSE_0, &response0))) {
    return err_code;
  }
  cmd_ctr = (response0 & RSP0_CMD_CTR);

  while (count < MAX_RETRIES) {
    if ((err_code = wait_until_ready_())) {
      return err_code;
    }
    if ((err_code = read_register_(RESPONSE_0, &response0))) {
      return err_code;
    }
    if ((response0 & RSP0_CMD_CTR) == cmd_ctr) {
      break;
    } else {
      cmd_ctr = (response0 & RSP0_CMD_CTR);
    }
    count++;
  }
  return cmd_ctr;
};

uint16_t Si1151Component::check_cmd_ctr_(uint8_t cmd_ctr) {
  // return values:
  // !0: counter wasn't incremented
  // =0: cmd_ctr has been incremented
  int const MAX_RETRIES = 5;
  uint8_t count = 0;
  uint8_t response0;
  i2c::ErrorCode err_code;

  count = 0;
  while (count < MAX_RETRIES) {
    if ((err_code = read_register_(RESPONSE_0, &response0))) {
      return err_code;
    }
    //// TODO should I actually test if counter is incremented (modulo), not just different?
    if ((response0 & RSP0_CMD_CTR) != cmd_ctr) {
      break;
    }
    count++;
  }
  return 0;
};

int16_t Si1151Component::send_command_(uint8_t cmd_code) {
  // return results:
  //  !0: failed to send command
  //  =0: successful command
  // all commands (except RESET_CMD_CTR and RESET_SW) increment CMD_CTR field in RESPONSE0
  int const MAX_RETRIES = 5;
  i2c::ErrorCode err_code;
  uint8_t response0;
  uint8_t cmd_ctr;
  uint16_t retval;

  if ((retval = get_cmd_ctr_())) {
    return retval;
  }
  cmd_ctr = retval;

  uint8_t packet[2] = {COMMAND, cmd_code};
  if ((err_code = this->write(packet, sizeof(packet)))) {
    ESP_LOGE(TAG, "send_command_: write COMMAND failed");
    return err_code;
  }
  return check_cmd_ctr(cmd_ctr);
};

int16_t Si1151Component::read_parameter_(uint8_t addr, uint8_t &param) {
  // return values:
  // !0: error in reading parameter
  // =0: successful read
  uint8_t buffer;
  int16_t retval;

  if ((retval = send_command_(PARAM_QUERY_CMD | (addr & RSP1_PARAM_ADDR)))) {
    return retval;
  }
  if ((i2c::ErrorCode err_code = read_register_(RESPONSE_1, &buffer))) {
    return err_code;
  }
  param = buffer;
  return 0;
};

int16_t Si1151Component::set_parameter_(uint8_t addr, uint8_t val) {
  // return values:
  // !0: failed to set parameter
  // =0: successfully set parameter
  uint8_t cmd_ctr;
  uint8_t response0;
  uint16_t retval;
  i2c::ErrorCode err_code;

  if ((retval = get_cmd_ctr_())) {
    return retval;
  }
  cmd_ctr = retval;
  if ((retval = write_register_(HOSTIN_0, val))) {
    return retval;
  }
  if ((retval = send_command_(PARAM_SET_CMD | (addr & PARAM_ADDR)))) {
    return retval;
  }
  return check_cmd_ctr(cmd_ctr);
};

int16_t Si1151Component::force() {
  return send_command_(FORCE_CMD);
};

//  The data (2 or 3 bytes) for all selected channels is packed into HOSTOUT?
// registers in order. 
//  The data for a channel is only considered valid if the channel's associated
// IRQ bit is set in the IRQ_STATUS register (values should be considered invalid
// if the bit is clear).
//  The IRQ_STATUS register is cleared on being read.
//  Not connecting INT signal. using Mode 1 (i.e., interrupt on each sample), so
// set THRESH_EN field in ADCPOST? registers to 0.
// "In an ALS measurement, two measurements are always taken and added together."

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
