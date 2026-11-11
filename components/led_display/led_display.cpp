#include "led_display.h"
#include "esphome/core/application.h"

namespace esphome {
namespace led_display {

void LedDisplayComponent::setup() {
  gpio_set_direction(ROW_BIT_0, GPIO_MODE_OUTPUT);
  gpio_set_direction(ROW_BIT_1, GPIO_MODE_OUTPUT);
  gpio_set_direction(ROW_BIT_2, GPIO_MODE_OUTPUT);

  gpio_set_direction(GREEN_LEDS_ENB, GPIO_MODE_OUTPUT);
  gpio_set_level(GREEN_LEDS_ENB, 1);

  gpio_set_direction(RED_LEDS_ENB, GPIO_MODE_OUTPUT);
  gpio_set_level(RED_LEDS_ENB, 1);

  gpio_set_direction(COL_DATA, GPIO_MODE_OUTPUT);
  gpio_set_level(COL_DATA, 0);

  gpio_set_direction(COL_STROBE, GPIO_MODE_OUTPUT);
  gpio_set_level(COL_STROBE, 0);

  gpio_set_direction(COL_CLOCK, GPIO_MODE_OUTPUT);
  gpio_set_level(COL_CLOCK, 0);

  // all rows off (row select 7 is unused, both color enables are active-low)
  REG_WRITE(GPIO_OUT_W1TS_REG, (ROW_BITS_MASK | ENABLES_MASK));

  // init the framebuffer to the size of the display and all black pixels
  this->frameBuffer_ = FrameBuffer_t(this->get_height_internal(),
                                    FrameBufferColumns_t(this->get_width_internal(),
                                                         LedColor_t{}));
  this->scrollOffset_ = 0;
  this->lastScroll_ = millis();

  this->currentColor_ = COLORS[1];       // defaults to Red
  this->currentFont_ = this->fonts_[0];  // defaults to first font (MT_PIXEL_5x7)

  assert(this->numFonts_ > 0);

  this->composeScanBuffer_();
  this->displayOn_ = true;

  // refresh the display from a 1MHz hardware timer, independent of the main loop
  gptimer_config_t timerConfig = {};
  timerConfig.clk_src = GPTIMER_CLK_SRC_DEFAULT;
  timerConfig.direction = GPTIMER_COUNT_UP;
  timerConfig.resolution_hz = 1000000;
  gptimer_event_callbacks_t callbacks = {};
  callbacks.on_alarm = LedDisplayComponent::onTimerAlarm_;
  gptimer_alarm_config_t alarm = {};
  alarm.alarm_count = SCAN_PHASE_US;
  if ((gptimer_new_timer(&timerConfig, &this->timer_) != ESP_OK) ||
      (gptimer_register_event_callbacks(this->timer_, &callbacks, this) != ESP_OK) ||
      (gptimer_set_alarm_action(this->timer_, &alarm) != ESP_OK) ||
      (gptimer_enable(this->timer_) != ESP_OK) ||
      (gptimer_start(this->timer_) != ESP_OK)) {
    ESP_LOGE(TAG, "Failed to start refresh timer");
    this->mark_failed();
    return;
  }

  ESP_LOGV(TAG, "setup complete");
};

void LedDisplayComponent::dump_config() {
  ESP_LOGCONFIG(TAG,
                "LED_DISPLAY:\n"
                "  Height: %u\n"
                "  Width: %u\n"
                "  Intensity: %u\n"
                "  Brightness: %u usec/row\n"
                "  Refresh Rate: %u Hz\n"
                "  Flip X: %u\n"
                "  Scrolling On: %u\n"
                "  Scroll Mode: %u\n"
                "  Scroll Speed: %u\n"
                "  Scroll Dwell: %u\n"
                "  Scroll Delay: %u",
                this->get_height_internal(),
                this->get_width_internal(),
                this->intensity_, this->brightness_,
                (unsigned) (1000000 / (SCAN_PHASE_US * NUM_SCAN_PHASES)),
                this->flipX_,
                this->scrollingOn_, this->scrollMode_,
                this->scrollSpeed_, this->scrollDwell_,
                this->scrollDelay_);
  //// TODO fill in more info -- e.g., LOG_PIN()????
  LOG_UPDATE_INTERVAL(this);
};

LedColor_t LedDisplayComponent::colorToLedColor(Color color) {
  // quantize RGB565 color value to the available colors
  uint32_t distBlack = (color.red * color.red) + 
                       (color.green * color.green) +
                       (color.blue * color.blue);
  uint32_t distRed = ((color.red - 255) * (color.red - 255)) +
                     (color.green * color.green) +
                     (color.blue * color.blue);
  uint32_t distGreen = (color.red * color.red) + 
                       ((color.green - 255) * (color.green - 255)) +
                       (color.blue * color.blue);
  uint32_t distAmber = ((color.red - 255) * (color.red - 255)) +
                       ((color.green - 191) * (color.green - 191)) +
                       (color.blue * color.blue);

  uint32_t minDist = distBlack;
  uint8_t ledColor = this->background_;

  if (distRed < minDist) {
    minDist = distRed;
    ledColor = RED_LED_COLOR;
  }
  if (distGreen < minDist) {
    minDist = distGreen;
    ledColor = GREEN_LED_COLOR;
  }
  if (distAmber < minDist) {
    minDist = distAmber;
    ledColor = AMBER_LED_COLOR;
  }
  return ledColor;
};

Color LedDisplayComponent::LedColorToColor(LedColor_t ledColor) {
  Color color = Color(0, 0, 0);

  switch (ledColor) {
  case (RED_LED_COLOR):
    color = Color(255, 0, 0);
    break;
  case (GREEN_LED_COLOR):
    color = Color(0, 255, 0);
    break;
  case (AMBER_LED_COLOR):
    color = Color(255, 255, 0);
    break;
  case (BLACK_LED_COLOR):
    color = Color(0, 0, 0);
    break;
  default:
    ESP_LOGE(TAG, "Invalid LED color: %d", ledColor);
  }
  return color;
};

uint32_t LedDisplayComponent::getStringWidth_(Font *font, const char *str) {
  int width = 0, xOffset = 0, baseline = 0, height = 0;
  font->measure(str, &width, &xOffset, &baseline, &height);
  ESP_LOGV(TAG, "String: '%s', width=%d, xOffset=%d, baseline=%d, height=%d",
           str, width, xOffset, baseline, height);
  return width;
};

void LedDisplayComponent::draw_absolute_pixel_internal(int x, int y, Color color) {
  // write pixel into framebuffer
  // N.B. Color is an RGB565 value stored in a 32b struct defined in esphome/core/color.h
  LedColor_t ledColor = this->colorToLedColor(color);

  if ((x + 1) > (int)this->frameBuffer_[0].size()) {
    // expand width (# of cols) of each of the framebuffer's rows
    for (int row = 0; (row < this->get_height_internal()); row++) {
      this->frameBuffer_[row].resize((x + 1), this->background_);
    }
  }

  // don't draw if pixel is outside the display's other bounds
  if ((y >= this->get_height_internal()) || (y < 0) || (x < 0)) return;

  this->frameBuffer_[y][x] = ledColor;
  ESP_LOGVV(TAG, "Draw Abs: x=%d, y=%d, c=0x%x (0x%x)", x, y, color, ledColor);
};

void LedDisplayComponent::update() {
  this->clear();

  if (this->writerLocal_.has_value()) {
    (*this->writerLocal_)(*this);
    ESP_LOGVV(TAG, "Executed Lambda drawing function");
  }
  ESP_LOGVV(TAG, "Update indicated, framebuffer cleared, and given lambda executed");
};

void LedDisplayComponent::loop() {
  // N.B. LED refresh happens in the timer ISR; this only advances scrolling
  //      and hands newly composed frames to the ISR.
  const uint32_t now = App.get_loop_component_start_time();
  const uint32_t msecSinceLastScroll = (now - this->lastScroll_);
  const uint16_t bufferWidth = this->frameBuffer_[0].size();

  if (this->scrollingOn_ && (bufferWidth > this->get_width_internal())) {
    if ((this->scrollOffset_ == 0) && (msecSinceLastScroll < this->scrollDelay_)) {
      // waiting at the start of the text
    } else if ((this->scrollMode_ == ScrollMode::STOP) &&
               ((this->scrollOffset_ + this->get_width_internal()) >= bufferWidth)) {
      // end of text reached -- dwell, then snap back to the start
      if (msecSinceLastScroll >= this->scrollDwell_) {
        ESP_LOGV(TAG, "Dwell passed. Resetting to start of text.");
        this->scrollOffset_ = 0;
        this->lastScroll_ = now;
        this->dirty_ = true;
      }
    } else if (msecSinceLastScroll >= this->scrollSpeed_) {
      this->scrollOffset_ = ((this->scrollOffset_ + 1) % bufferWidth);
      this->lastScroll_ = now;
      this->dirty_ = true;
    }
  }

  // hand a new frame to the ISR (if it has picked up the previous one)
  if (this->dirty_ && !this->swapPending_) {
    this->composeScanBuffer_();
  }
};

void LedDisplayComponent::clear() {
  // reset the width of each of the framebuffer's rows and clear them
  for (int row = 0; (row < this->get_height_internal()); row++) {
    this->frameBuffer_[row].clear();
  }
  // new content starts from the beginning
  this->scrollOffset_ = 0;
  this->lastScroll_ = millis();
  this->dirty_ = true;
  ESP_LOGV(TAG, "Cleared the framebuffer");
};

uint8_t LedDisplayComponent::printLED(uint8_t startPos, const char *str) {
  int xPos = 0;
  size_t strIndx = 0;
  std::string strBuf;

  Font *startFont = this->currentFont_;
  LedColor_t startColor = this->currentColor_;

  this->clear();

  // look for escape sequences of the form: "\033[<fontNum>;<colorNum>m"
  while (str[strIndx] != '\0') {
    if (str[strIndx] == '\\') {
      strIndx++;
      if (str[strIndx++] != '[') {
        ESP_LOGW(TAG, "Invalid escape sequence, missing '[': (%c)", str[strIndx - 1]);
        continue;
      }
      auto fontNum = (str[strIndx++] - '0');
      if ((fontNum < 0) || (fontNum >= this->numFonts_)) {
        ESP_LOGW(TAG, "Invalid font number: (%c)", str[strIndx - 1]);
        continue;
      }
      if (str[strIndx++] != ';') {
        ESP_LOGW(TAG, "Invalid escape sequence, missing ';': (%c)", str[strIndx - 1]);
        continue;
      }
      auto colorNum = (str[strIndx++] - '0');
      if ((colorNum < 0) || (colorNum >= MAX_NUM_COLORS)) {
        ESP_LOGW(TAG, "Invalid color number: (%c)", str[strIndx - 1]);
        continue;
      }
      if (str[strIndx++] != 'm') {
        ESP_LOGW(TAG, "Invalid escape sequence, missing 'm': (%c)", str[strIndx - 1]);
        continue;
      }
      if (!strBuf.empty()) {
        this->print(xPos, 0, this->currentFont_, LedColorToColor(this->currentColor_),
                    esphome::display::TextAlign::TOP_LEFT, strBuf.c_str(),
                    LedColorToColor(this->background_));
        auto x = this->getStringWidth_(this->currentFont_, strBuf.c_str());
        ESP_LOGD(TAG, "> xPos=%u, x=%u", xPos, x);  //// TMP TMP TMP
        xPos += x;
        ESP_LOGD(TAG, "printLED: font=%u, color=%u, xPos=%u, str=%s)", 
                 this->currentFont_, this->currentColor_, xPos, strBuf.c_str());
        strBuf.clear();
      }
      this->currentFont_ = this->fonts_[fontNum];
      this->currentColor_ = COLORS[colorNum];
      continue;
    }

    // add char to string buffer
    strBuf += str[strIndx++];
  }
  if (!strBuf.empty()) {
    this->print(xPos, 0, this->currentFont_, LedColorToColor(this->currentColor_),
                esphome::display::TextAlign::TOP_LEFT, strBuf.c_str(),
                LedColorToColor(this->background_));
    auto x = this->getStringWidth_(this->currentFont_, strBuf.c_str());
    ESP_LOGD(TAG, "> xPos=%u, x=%u", xPos, x);  //// TMP TMP TMP
    xPos += x;
    ESP_LOGD(TAG, "final printLED: font=%u, color=%u, xPos=%u, str=%s)", 
             this->currentFont_, this->currentColor_, xPos, strBuf.c_str());
  }

  this->currentFont_ = startFont;
  this->currentColor_ = startColor;
  ESP_LOGD(TAG, "exit printLED: currentFont=%u, currentColor=%u",
           this->currentFont_, this->currentColor_);
  return xPos;
};

void LedDisplayComponent::composeScanBuffer_() {
  // render the visible window of the framebuffer into the back scan buffer,
  // in the order the bits get shifted out, with flip and invert applied
  const int width = this->get_width_internal();
  const int height = this->get_height_internal();
  const size_t bufferWidth = this->frameBuffer_[0].size();
  const bool wrap = (this->scrollingOn_ && (bufferWidth > (size_t)width));
  const uint8_t on = (this->invert_ ? 0 : 1);
  auto &buf = this->scanBuf_[this->shownBuf_ ^ 1];

  for (int r = 0; (r < height); r++) {
    const auto &line = this->frameBuffer_[this->flipX_ ? ((height - 1) - r) : r];
    for (int c = 0; (c < width); c++) {
      size_t x = this->flipX_ ? c : ((width - 1) - c);
      if (wrap) {
        x = ((x + this->scrollOffset_) % bufferWidth);
      }
      const LedColor_t pixel = ((x < line.size()) ? line[x] : this->background_);
      buf[(r * 2) + 0][c] = ((pixel & RED_LED_COLOR) ? on : !on);
      buf[(r * 2) + 1][c] = ((pixel & GREEN_LED_COLOR) ? on : !on);
    }
  }

  // make sure the buffer is written before the ISR can see the swap request
  __asm__ __volatile__("" ::: "memory");
  this->swapPending_ = true;
  this->dirty_ = false;
};

static inline void spinCycles_(uint32_t cycles) {
  const uint32_t start = esp_cpu_get_cycle_count();
  while ((esp_cpu_get_cycle_count() - start) < cycles) {
  }
}

bool LedDisplayComponent::onTimerAlarm_(gptimer_handle_t timer,
                                        const gptimer_alarm_event_data_t *edata,
                                        void *arg) {
  auto *self = static_cast<LedDisplayComponent *>(arg);
  gptimer_alarm_config_t alarm = {};
  alarm.alarm_count = (edata->alarm_value + self->refreshPhase_());
  gptimer_set_alarm_action(timer, &alarm);
  return false;
}

uint32_t LedDisplayComponent::refreshPhase_() {
  // called from the timer ISR -- alternates between turning a row on and off;
  // returns the number of usecs until the next call
  const uint32_t onTime = this->brightness_;

  // turn the current row off
  REG_WRITE(GPIO_OUT_W1TS_REG, (ROW_BITS_MASK | ENABLES_MASK));
  if (this->ledsOn_) {
    this->ledsOn_ = false;
    return (SCAN_PHASE_US - SHIFT_TIME_US - onTime);
  }

  if (this->phase_ == 0) {
    // start of a new frame -- switch to the newest buffer, if there is one
    if (this->swapPending_) {
      this->shownBuf_ ^= 1;
      this->swapPending_ = false;
    }
  }
  const uint8_t phase = this->phase_;
  this->phase_ = ((phase + 1) % NUM_SCAN_PHASES);

  if (!this->displayOn_) {
    return SCAN_PHASE_US;
  }

  // shift in the next row's data and latch it
  const uint8_t *data = this->scanBuf_[this->shownBuf_][phase];
  for (int c = 0; (c < LED_DISP_WIDTH); c++) {
    REG_WRITE(GPIO_OUT_W1TC_REG, ((1UL << COL_CLOCK) | (data[c] ? 0 : (1UL << COL_DATA))));
    if (data[c]) {
      REG_WRITE(GPIO_OUT_W1TS_REG, (1UL << COL_DATA));
    }
    spinCycles_(SETUP_CYCLES);
    REG_WRITE(GPIO_OUT_W1TS_REG, (1UL << COL_CLOCK));
    spinCycles_(PULSE_CYCLES);
  }
  REG_WRITE(GPIO_OUT_W1TC_REG, (1UL << COL_CLOCK));
  REG_WRITE(GPIO_OUT_W1TS_REG, (1UL << COL_STROBE));
  spinCycles_(PULSE_CYCLES);
  REG_WRITE(GPIO_OUT_W1TC_REG, (1UL << COL_STROBE));

  // select the row, let the decoder settle, then enable the row's color
  const uint32_t row = (phase >> 1);
  const uint32_t rowBits = (((row & 0x01) ? (1UL << ROW_BIT_0) : 0) |
                            ((row & 0x02) ? (1UL << ROW_BIT_1) : 0) |
                            ((row & 0x04) ? (1UL << ROW_BIT_2) : 0));
  REG_WRITE(GPIO_OUT_W1TC_REG, (ROW_BITS_MASK & ~rowBits));
  spinCycles_(SETUP_CYCLES);
  REG_WRITE(GPIO_OUT_W1TC_REG, ((phase & 0x01) ? (1UL << GREEN_LEDS_ENB) : (1UL << RED_LEDS_ENB)));
  this->ledsOn_ = true;
  // timed from the start of this phase, so shifting time is absorbed by SHIFT_TIME_US
  return (SHIFT_TIME_US + onTime);
}

}  // namespace led_display
}  // namespace esphome
