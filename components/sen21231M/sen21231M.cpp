#include "sen21231M.h"
#include "esphome/core/log.h"

namespace esphome {
namespace sen21231M_sensor {


static const char *const TAG = "sen21231M_sensor.sensor";


void Sen21231MSensor::setup() {
  _usps = new USPS();

  int8_t n = -1;
  while (n < 0) {
    USPSface_t faces[1];
    delay(500);
    _usps = new USPS();
    n = _usps->getFaces(faces, 1);
    if (trys-- <= 0) {
        ESP_LOGE(TAG, "Failed to initialize the SEN21231M");
        //// TODO reboot?
        return;
    }
  }
}

void Sen21231MSensor::update() {
  USPSface_t faces[USPS_MAX_FACES];
  int8_t numFaces;

  int8_t n = _usps->getFaces(faces, USPS_MAX_FACES);
  numFaces = n;
  for (int i = 0; (i < n); i++) {
    if (!faces[i].isFacing || (faces[i].boxConfidence < MIN_CONFIDENCE)) {
      numFaces--;
    }
  }
  if (numFaces < 0) {
    ESP_LOGE(TAG, "Failed to read SEN21231M, resetting...");
    _usps = new USPS();
    return;
  }

  ESP_LOGD(TAG, "'%s': # Faces Detected=%d", this->get_name().c_str(), numFaces);
  this->publish_state(numFaces);
}

void Sen21231MSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "SEN21231M:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SEN21231M failed!");
  }
  ESP_LOGI(TAG, "SEN21231M: %s", this->is_failed() ? "FAILED" : "OK");
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace sen21231M_sensor
}  // namespace esphome
