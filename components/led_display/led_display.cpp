#include "led_display.h"
#include "esphome/core/application.h"

namespace esphome {
namespace led_display {


void LedDisplayComponent::setup() {
  pinMode(ROW_BIT_0, OUTPUT);
  pinMode(ROW_BIT_1, OUTPUT);
  pinMode(ROW_BIT_2, OUTPUT);

  pinMode(GREEN_LEDS_ENB, OUTPUT);
  digitalWrite(GREEN_LEDS_ENB, HIGH);

  pinMode(RED_LEDS_ENB, OUTPUT);
  digitalWrite(RED_LEDS_ENB, HIGH);

  pinMode(COL_DATA, OUTPUT);
  digitalWrite(COL_DATA, LOW);

  pinMode(COL_STROBE, OUTPUT);
  digitalWrite(COL_STROBE, LOW);

  pinMode(COL_CLOCK, OUTPUT);
  digitalWrite(COL_CLOCK, LOW);

  this->disableRows_();

  // init the framebuffer to the size of the display and all black pixels
  this->frameBuffer_ = FrameBuffer_t(this->get_height_internal(),
                                    FrameBufferColumns_t(this->get_width_internal(),
                                                         LedColor_t{}));
  this->update_ = false;
  this->oldBufferWidth_ = 0;

  this->lastScroll_ = 0;
  this->stepsLeft_ = 0;
  this->lastLoop_ = App.get_loop_component_start_time();

  this->display_();

  this->displayOn_ = true;

  ESP_LOGV(TAG, "setup complete");
};

void LedDisplayComponent::dump_config() {
  ESP_LOGCONFIG(TAG,
                "LED_DISPLAY:\n"
                "  Height: %u\n"
                "  Width: %u\n"
                "  Intensity: %u\n"
                "  Brightness: %u\n"
                "  Flip X: %u\n"
                "  Scrolling On: %u\n"
                "  Scroll Mode: %u\n"
                "  Scroll Speed: %u\n"
                "  Scroll Dwell: %u\n"
                "  Scroll Delay: %u",
                this->get_height_internal(),
                this->get_width_internal(),
                this->intensity_, this->brightness_,
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
  const uint32_t now = App.get_loop_component_start_time();
  const uint32_t msecSinceLastLoop = (now - this->lastLoop_);
  //ESP_LOGVV(TAG, "Refresh rate: %.1f fps (%u msec)", (1000.0 / msecSinceLastLoop), msecSinceLastLoop);
  this->lastLoop_ = now;
  const uint32_t msecSinceLastScroll = (now - this->lastScroll_);

  // call display if the buffer has changed size since last update
  const size_t bufferWidth = this->frameBuffer_[0].size();
  if ((bufferWidth >= (this->oldBufferWidth_ + 3)) || (bufferWidth <= (this->oldBufferWidth_ - 3))) {
    ESP_LOGV(TAG, "Buffer size changed %d to %d", this->oldBufferWidth_, bufferWidth);
    this->stepsLeft_ = 0;
    this->display_();
    this->oldBufferWidth_ = bufferWidth;
  }

  // check if scroll isn't needed (turned off, or framebuffer is smaller than the display width)
  if (!this->scrollingOn_ || (bufferWidth <= this->get_width_internal())) {
    ESP_LOGVV(TAG, "No need to scroll or scroll is off");
    this->display_();
    return;
  }

  // check if scrolling is to be started and enough delay time has elapsed
  if ((this->stepsLeft_ == 0) && (msecSinceLastScroll < this->scrollDelay_)) {
    ESP_LOGVV(TAG, "At first step. Waiting for scroll delay (%u < %u)",
              msecSinceLastScroll, this->scrollDelay_);
    this->display_();
    return;
  }

  if (this->scrollMode_ == ScrollMode::STOP) {
    // scroll in stop mode, check if this is at the end of the line
    if ((this->stepsLeft_ + this->get_width_internal()) == (bufferWidth + 1)) {
      // end of the line, see if we're done with dwell time
      if (msecSinceLastScroll < this->scrollDwell_) {
        ESP_LOGVV(TAG, "Dwell time at end of string in case of stop at end. Step %d, since last scroll %d, dwell %d.",
                  this->stepsLeft_, msecSinceLastScroll, this->scrollDwell_);
        return;
      }
      ESP_LOGVV(TAG, "Dwell time passed. Continue scrolling.");
    }
  }

  // if got here, then still scrolling, check if ready to take the next step
  if (msecSinceLastScroll >= this->scrollSpeed_) {
    ESP_LOGVV(TAG, "Call to scroll left action; time since last scroll: %u msec, stepsLeft: %u",
              msecSinceLastScroll, this->stepsLeft_);
    this->lastScroll_ = now;
    this->scrollLeft_();
    this->display_();
  } else {
    ESP_LOGVV(TAG, "Not ready to do the next scroll step; %u", msecSinceLastScroll);
    this->display_();
  }
};

void LedDisplayComponent::clear() {
  // flag update needed and clear frame buffer
  this->update_ = true;

  // reset the width of each of the framebuffer's rows and clear them
  for (int row = 0; (row < this->get_height_internal()); row++) {
    this->frameBuffer_[row].resize(this->get_width_internal(), this->background_);
    this->frameBuffer_[row].clear();
  }
  ESP_LOGD(TAG, "Reset the framebuffer size and clear its rows");  //// TMP TMP TMP
};

uint8_t LedDisplayComponent::printLED(uint8_t startPos, const char *str) {
//  this->clear();
  ESP_LOGD(TAG, "printLED(): TBD");
  return 0; //// (startPos + (strlen(str) * LED_CHAR_WIDTH));
};

void LedDisplayComponent::scrollLeft_() {
  // define a lambda to rotate a line left by the given number of steps
  auto scroll = [&](std::vector<uint8_t> &line, uint16_t steps) {
    std::rotate(line.begin(), std::next(line.begin(), steps), line.end());
  };

  // scroll by circular rotating all rows left
  for (int row = 0; row < this->get_height_internal(); row++) {
    uint32_t sum = std::accumulate(this->frameBuffer_[row].begin(), this->frameBuffer_[row].end(), 0u);  //// TMP TMP TMP
    ESP_LOGVV(TAG, "scrollLeft Pre: %u, sum(row: %u) = %u", this->update_, row, sum);  //// TMP TMP TMP
    if (this->update_) {
      // update required, so append a black pixel to the end of the row to ensure the row's long enough
      this->frameBuffer_[row].push_back(this->background_);
      // circular rotate the row by one more than the number of steps left
      // because an update requires ????
      scroll(this->frameBuffer_[row],
             (this->stepsLeft_ + 1) % (this->frameBuffer_[row].size()));
    } else {
      // no update required, so just rotate the current row by one
      scroll(this->frameBuffer_[row], 1);
    }
    sum = std::accumulate(this->frameBuffer_[row].begin(), this->frameBuffer_[row].end(), 0u);  //// TMP TMP TMP
    ESP_LOGVV(TAG, "scrollLeft Post: sum(row: %u) = %u", row, sum);  //// TMP TMP TMP
  }
  this->update_ = false;
  this->stepsLeft_++;
  this->stepsLeft_ %= this->frameBuffer_[0].size();
  ESP_LOGVV(TAG, "Scrolled left; stepsLeft: %u", this->stepsLeft_);
};

void LedDisplayComponent::display_() {
  if (!this->displayOn_) return;
  // for each row and for both colors, shift in row data and latch it, then enable the row&color
  for (uint r = 0; (r < this->get_height_internal()); r++) {
    auto row = this->flipX_ ? ((this->get_height_internal() - 1) - r) : r;
    for (LedColor_t color : {RED_LED_COLOR, GREEN_LED_COLOR}) {
      this->shiftInPixels_(color, row);
      this->enableRow_(color, r);
      delayMicroseconds(this->brightness_);
      this->disableRows_();
    }
  }
};

void LedDisplayComponent::enableRow_(LedColor_t rowColor, uint rowNum) {
  assert(rowNum < this->get_height_internal());
  digitalWrite(ROW_BIT_0, (rowNum & 0x01));
  digitalWrite(ROW_BIT_1, (rowNum & 0x02));
  digitalWrite(ROW_BIT_2, (rowNum & 0x04));

  switch (rowColor) {
  case (GREEN_LED_COLOR):
    digitalWrite(GREEN_LEDS_ENB, LOW);
    digitalWrite(RED_LEDS_ENB, HIGH);
    break;
  case (RED_LED_COLOR):
    digitalWrite(GREEN_LEDS_ENB, HIGH);
    digitalWrite(RED_LEDS_ENB, LOW);
    break;
  default:
    ESP_LOGE(TAG, "Invalid row color: %u", rowColor);
  }
};

void LedDisplayComponent::disableRows_() {
    digitalWrite(GREEN_LEDS_ENB, HIGH);
    digitalWrite(RED_LEDS_ENB, HIGH);
    digitalWrite(ROW_BIT_0, HIGH);
    digitalWrite(ROW_BIT_1, HIGH);
    digitalWrite(ROW_BIT_2, HIGH);
};

void LedDisplayComponent::shiftInPixels_(LedColor_t rowColor, uint rowNum) {
  // clock in and latch all of the given row's pixel data
  //// FIXME make invert map BLK->AMB, RED->BLK, GRN->BLK????
  uint8_t hi = (this->invert_ ? 0 : 1);
  uint8_t lo = (this->invert_ ? 1 : 0);
  for (int c = 0; (c < this->get_width_internal()); c++) {
    auto col = this->flipX_ ? c : ((this->get_width_internal() - 1) - c);
    digitalWrite(COL_CLOCK, LOW);
    digitalWrite(COL_DATA, ((this->frameBuffer_[rowNum][col] & rowColor) ? hi : lo));
    digitalWrite(COL_CLOCK, HIGH);
  }

  // strobe to latch data -- desired color of LEDs in the row are now set
  digitalWrite(COL_STROBE, HIGH);
  digitalWrite(COL_STROBE, LOW);
};

}  // namespace led_display
}  // namespace esphome
