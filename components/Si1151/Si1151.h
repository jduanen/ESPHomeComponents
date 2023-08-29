#pragma once

#include <bitset>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"


namespace esphome {
namespace Si1151 {

enum Si1151UnitAddress {
  DEVICE_ADDRESS = 0x53
};

enum CommandCodes {
  RESET_CTR_CMD   = 0x00,  // aka NOP cmd, CMD_CTR = 0
  RESET_SW_CMD    = 0x01,  // reset, CMD_CTR = 0xF
  NEW_ADDR_CMD    = 0x02,  // set new I2C address

  FORCE_CMD       = 0x11,  // start measurements defined in CHAN_LIST, counter index must be 0 in MEASCONFIG?
  PAUSE_CMD       = 0x12,  // pauses measurements
  START_CMD       = 0x13,  // aka AUTO cmd, counter index in MEASCONFIG? must not be 0

  PARAM_QUERY_CMD = 0x40,  // read parameter at address (0x40 | PARAM_ADDRESS) into RESPONSE1
  PARAM_SET_CMD   = 0x80,  // write value in HOSTIN0 to parameter address (0x80 | PARAM_ADDRESS) and into RESPONSE1
};

enum RegisterAddress {
  PART_ID    = 0x00,  // R; Si1151: 0x51, Si1152: 0x52, Si1153: 0x53
  REV_ID     = 0x01,  // R; 0xMN: M=Major Rev., N=Minor Rev.
  MFR_ID     = 0x02,  // R; [7:5]=HW Rev., [4:0]=Filter, LED, and Module code
  INFO_0     = 0x03,
  INFO_1     = 0x04,
  HOSTIN_3   = 0x07,
  HOSTIN_2   = 0x08,
  HOSTIN_1   = 0x09,
  HOSTIN_0   = 0x0A,  // RW; value to write to param table with PARAM_SET cmd
  COMMAND    = 0x0B,  // RW; write command codes here
  IRQ_ENABLE = 0x0F,  // RW; enable interrupts for channels; [5:0]=IE5-0; WR: enable, RD: ready?
  RESPONSE_1 = 0x10,  // R; readback value from PARAM_QUERY or PARAM_SET
  RESPONSE_0 = 0x11,  // R; power state and command counter, resets to 0x2F; [7]=RUNNING, [6]=SUSPEND, [5]=SLEEP, [4]=CMD_ERR, [3:0]=CMD_CTR
  IRQ_STATUS = 0x12,  // CR; interrupt flag status for each channel
  HOSTOUT_0  = 0x13,  // R; ADC output data, 2 or 3 bytes per selected/active channel, active indicated in IRQ status
  HOSTOUT_1  = 0x14,
  HOSTOUT_2  = 0x15,
  HOSTOUT_3  = 0x16,
  HOSTOUT_4  = 0x17,
  HOSTOUT_5  = 0x18,
  HOSTOUT_6  = 0x19,
  HOSTOUT_7  = 0x1A,
  HOSTOUT_8  = 0x1B,
  HOSTOUT_9  = 0x1C,
  HOSTOUT_10 = 0x1D,
  HOSTOUT_11 = 0x1E,
  HOSTOUT_12 = 0x1F,
  HOSTOUT_13 = 0x20,
  HOSTOUT_14 = 0x21,
  HOSTOUT_15 = 0x22,
  HOSTOUT_16 = 0x23,
  HOSTOUT_17 = 0x24,
  HOSTOUT_18 = 0x25,
  HOSTOUT_19 = 0x26,
  HOSTOUT_20 = 0x27,
  HOSTOUT_21 = 0x28,
  HOSTOUT_22 = 0x29,
  HOSTOUT_23 = 0x2A,
  HOSTOUT_24 = 0x2B,
  HOSTOUT_25 = 0x2C
};

enum ParameterAddress {
  I2C_ADDR     = 0x00,
  CHAN_LIST    = 0x01,  // active channel list

  ADCCONFIG_0  = 0x02,  // Channel 0 setup
  ADCSENS_0    = 0x03,  // ...
  ADCPOST_0    = 0x04,
  MEASCONFIG_0 = 0x05,

  ADCCONFIG_1  = 0x06,  // Channel 1 setup
  ADCPOST_1    = 0x08,  // ...
  ADCSENS_1    = 0x07,
  MEASCONFIG_1 = 0x09,

  ADCCONFIG_2  = 0x0A,  // Channel 2 setup
  ADCSENS_2    = 0x0B,  // ...
  ADCPOST_2    = 0x0C,
  MEASCONFIG_2 = 0x0D,

  ADCCONFIG_3  = 0x0E,  // Channel 3 setup
  ADCSENS_3    = 0x0F,  // ...
  ADCPOST_3    = 0x10,
  MEASCONFIG_3 = 0x11,

  ADCCONFIG_4  = 0x12,  // Channel 4 setup
  ADCSENS_4    = 0x13,  // ...
  ADCPOST_4    = 0x14,
  MEASCONFIG_4 = 0x15,

  ADCCONFIG_5  = 0x16,  // Channel 6 setup
  ADCSENS_5    = 0x17,  // ...
  ADCPOST_5    = 0x18,
  MEASCONFIG_5 = 0x19,

  MEASRATE_H   = 0x1A,  // measure rate
  MEASRATE_L   = 0x1B,  // ...
  MEASCOUNT_0  = 0x1C,  // meascount
  MEASCOUNT_1  = 0x1D,  // ...
  MEASCOUNT_2  = 0x1E,

  LED1_A       = 0x1F,  // LED current settings
  LED1_B       = 0x20,  // ...
  LED2_A       = 0x21,
  LED2_B       = 0x22,
  LED3_A       = 0x23,
  LED3_B       = 0x24,

  THRESHOLD0_H = 0x25,  // threshold setup
  THRESHOLD0_L = 0x26,  // ...
  THRESHOLD1_H = 0x27,
  THRESHOLD1_L = 0x28,

  U_THRSHOLD_H = 0x29,  // upper threshold window setup
  U_THRSHOLD_L = 0x2A,  // ...

  BURST        = 0x2B,  // burst control

  L_THRSHOLD_H = 0x2C,  // upper threshold window setup
  L_THRSHOLD_L = 0x2D   // ...
};

enum Channels {
  IR_CHANNEL      = 0,
  VISIBLE_CHANNEL = 1,
  UV_CHANNEL      = 2
};


uint8_t const MAX_NUM_CHANNELS = 6;

// RESPONSE_0 bit indicies
uint8_t const RSP0_RUNNING_INDX = 7;
uint8_t const RSP0_SUSPEND_INDX = 6;
uint8_t const RSP0_SLEEP_INDX   = 5;
uint8_t const RSP0_CMD_ERR_INDX = 4;

// RESPONSE_0 CMD_ERR values
uint8_t const RSP0_INVALID_CMD_ERR  = 0x0;
uint8_t const RSP0_INVALID_ADDR_ERR = 0x1;
uint8_t const RSP0_SATURATION_ERR   = 0x2;
uint8_t const RSP0_BUF_OVERFLOW_ERR = 0x3;

// RESPONSE_0 bit masks
uint8_t const RSP0_RUNNING = 0x80;  // set after reset
uint8_t const RSP0_SUSPEND = 0x40;  // set when ADC measurement being made
uint8_t const RSP0_SLEEP   = 0x20;  // set after initialization done
uint8_t const RSP0_CMD_ERR = 0x10;  // cleared by RESET_SW_CMD and RESET_CTR_CMD, set on error

// CMD_ERR=0: increments after each successful cmd
//            reset to 0 on RESET_CTR_CMD command
//            set to 0xF on RESET_SW_CMD command
uint8_t const RSP0_CMD_CTR = 0x0F;

// CMD_ERR=1: 0x0=Invalid Command
//            0x1=Parameter access to invalid location
//            0x2=Saturation of ADC or overflow of accumulation
//            0x3=Output buffer overflow (e.g., Burst > 26B)
uint8_t const RSP0_ERR_CODE = 0x0F

// RESPONSE_1 bit masks
uint8_t const RSP1_PARAM_ADDR = 0x3F;

// Parameter address mask
uint8_t const PARAM_ADDR = 0x3F;


class Si1151Component : public PollingComponent, public i2c::I2CDevice {
  public:
    void set_ir_sensor(sensor::Sensor *ir_sensor);
    void set_visible_sensor(sensor::Sensor *visible_sensor);
    void set_uv_sensor(sensor::Sensor *uv_sensor);

    void setup() override;
    float get_setup_priority() const override {
      return setup_priority::DATA;
    };
    void dump_config() override;
    void update() override;

  protected:
    sensor::Sensor *ir_sensor_{nullptr};
    sensor::Sensor *visible_sensor_{nullptr};
    sensor::Sensor *uv_sensor_{nullptr};

    i2c::ErrorCode read_register_(uint8_t reg, uint8_t &data);
    i2c::ErrorCode write_register_(uint8_t reg, uint8_t data);

    int16_t Si1151Component::wait_until_ready_(void);
    int16_t Si1151Component::hard_reset_(void);
    int16_t Si1151Component::reset_(void);

    uint16_t Si1151Component::get_cmd_ctr_(void);
    uint16_t Si1151Component::check_cmd_ctr_(uint8_t cmd_ctr);

    int16_t Si1151Component::send_command_(uint8_t cmd_code);
    int16_t Si1151Component::read_parameter_(uint8_t addr, uint8_t &param);
    int16_t Si1151Component::set_parameter_(uint8_t addr, uint8_t val);

    int16_t Si1151Component::force();

    int32_t read_IR_(void);
    int32_t read_visible_(void);
    float read_UV_(void);

  private:
    std::bitset<MAX_NUM_CHANNELS> enabled_channels_;
    uint8_t num_channels_ = 0;
};

}  // namespace Si1151
}  // namespace esphome
