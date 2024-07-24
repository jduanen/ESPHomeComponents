#include "sen21231.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sen21231M_sensor {

static const char *const TAG = "sen21231M_sensor.sensor";

void Sen21231MSensor::update() { this->read_data_(); }

void Sen21231MSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "SEN21231M:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SEN21231M failed!");
  }
  ESP_LOGI(TAG, "SEN21231M: %s", this->is_failed() ? "FAILED" : "OK");
  LOG_UPDATE_INTERVAL(this);
}

void Sen21231MSensor::read_data_() {
  person_sensor_results_t results;
  this->read_bytes(PERSON_SENSOR_I2C_ADDRESS, (uint8_t *) &results, sizeof(results));
  ESP_LOGD(TAG, "SEN21231M: %d faces detected", results.num_faces);
  this->publish_state(results.num_faces);
  if (results.num_faces == 1) {
    ESP_LOGD(TAG, "SEN21231M: is facing towards camera: %d", results.faces[0].is_facing);
  }
}

}  // namespace sen21231M_sensor
}  // namespace esphome
