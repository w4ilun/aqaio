#include "aqaio.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace aqaio {

static const char *const TAG = "aqaio";

// ─── Bitmaps (from layout) ──────────────────────────────────────────────────

static const unsigned char PROGMEM image_sad_face_aq_bits[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,
    0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,
    0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,
    0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,
    0x3f,0xf0,0x00,0x00,0x00,0x03,0xff,0x00,0x3f,0xf0,0x00,0x00,0x00,0x03,0xff,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xf0,0x00,0x00,0x00,
    0x00,0x00,0x0f,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x3c,0x00,0x00,0x00,
    0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,
    0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,
    0x00,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM image_happy_face_aq_bits[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,
    0x3f,0xcc,0x00,0x00,0x00,0x0f,0xf3,0x00,0x3f,0xcc,0x00,0x00,0x00,0x0f,0xf3,0x00,
    0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,
    0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,
    0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,
    0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,
    0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,
    0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,
    0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x00,0x00,0x00,
    0x00,0x00,0x00,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00
};

static const unsigned char PROGMEM image_weather_humidity_bits[] = {
    0x04,0x00,0x04,0x00,0x0c,0x00,0x0e,0x00,0x1e,0x00,0x1f,0x00,
    0x3f,0x80,0x3f,0x80,0x7e,0xc0,0x7f,0x40,0xff,0x60,0xff,0xe0,
    0x7f,0xc0,0x7f,0xc0,0x3f,0x80,0x0f,0x00
};

static const unsigned char PROGMEM image_weather_temperature_bits[] = {
    0x1c,0x00,0x22,0x02,0x2b,0x05,0x2a,0x02,0x2b,0x38,0x2a,0x60,
    0x2b,0x40,0x2a,0x40,0x2a,0x60,0x49,0x38,0x9c,0x80,0xae,0x80,
    0xbe,0x80,0x9c,0x80,0x41,0x00,0x3e,0x00
};

// ─── Air quality thresholds ─────────────────────────────────────────────────

static const uint16_t AQ_CO2_MAX = 1000;
static const float AQ_VOC_MAX = 150.0f;
static const float AQ_NOX_MAX = 1.0f;
static const float AQ_PM_MAX = 12.0f;

// ─── Helper Functions ───────────────────────────────────────────────────────

float AQAIOComponent::to_display_temp_(float temp_c) {
  if (this->use_fahrenheit_) {
    return temp_c * 9.0f / 5.0f + 32.0f;
  }
  return temp_c;
}

const char *AQAIOComponent::temp_unit_str_() {
  return this->use_fahrenheit_ ? "F" : "C";
}

void AQAIOComponent::format_elapsed_(unsigned long elapsed_ms, char *buf, size_t buf_len) {
  unsigned long secs = elapsed_ms / 1000;
  if (secs < 60) {
    snprintf(buf, buf_len, "%lus ago", secs);
  } else if (secs < 3600) {
    unsigned long mins = secs / 60;
    snprintf(buf, buf_len, "%lum ago", mins);
  } else {
    unsigned long hrs = secs / 3600;
    unsigned long mins = (secs % 3600) / 60;
    snprintf(buf, buf_len, "%luh%lum ago", hrs, mins);
  }
}

// ─── Display Rendering ─────────────────────────────────────────────────────

void AQAIOComponent::draw_splash_() {
  display_->fillScreen(GxEPD_BLACK);
  display_->setTextColor(GxEPD_WHITE);
  display_->setFont(&Org_01);
  display_->setTextSize(4);
  display_->setCursor(30, 90);
  display_->print("AQAIO");
  display_->setTextSize(2);
  display_->setCursor(20, 120);
  display_->print("Initializing...");
  display_->display(false);
}

void AQAIOComponent::draw_display_() {
  display_->fillScreen(GxEPD_BLACK);
  display_->setTextColor(GxEPD_WHITE);
  display_->setTextWrap(false);
  display_->setFont(&Org_01);

  char buf[16];

  // ── Temperature value ───────────────────────────────────────────────
  display_->setTextSize(3);
  display_->setCursor(22, 24);
  if (sensor_data_.valid) {
    float disp_temp = to_display_temp_(sensor_data_.temperature);
    dtostrf(disp_temp, 4, 1, buf);
    char *p = buf;
    while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }
  display_->setTextSize(2);
  display_->print(temp_unit_str_());

  // ── Temperature icon ────────────────────────────────────────────────
  display_->drawBitmap(5, 11, image_weather_temperature_bits, 16, 16, GxEPD_WHITE);

  // ── Humidity icon ───────────────────────────────────────────────────
  display_->drawBitmap(111, 11, image_weather_humidity_bits, 11, 16, GxEPD_WHITE);

  // ── Humidity value ──────────────────────────────────────────────────
  display_->setTextSize(3);
  display_->setCursor(125, 24);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.humidity, 4, 1, buf);
    char *p = buf;
    while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }

  // ── Humidity unit ───────────────────────────────────────────────────
  display_->setTextSize(2);
  display_->setCursor(185, 25);
  display_->print("%");

  // ── Divider line ────────────────────────────────────────────────────
  display_->fillRect(5, 32, 190, 3, GxEPD_WHITE);

  // ── CO2 label ───────────────────────────────────────────────────────
  display_->setTextSize(2);
  display_->setCursor(5, 61);
  display_->print("CO2");

  // ── CO2 value ───────────────────────────────────────────────────────
  display_->setTextSize(4);
  display_->setCursor(65, 59);
  if (sensor_data_.valid) {
    display_->print(sensor_data_.co2);
  } else {
    display_->print("----");
  }

  // ── CO2 unit ────────────────────────────────────────────────────────
  int16_t ppm_x = display_->getCursorX() + 5;
  display_->setTextSize(2);
  display_->setCursor(ppm_x, 61);
  display_->print("PPM");

  // ── VOC label ───────────────────────────────────────────────────────
  display_->setTextSize(2);
  display_->setCursor(5, 85);
  display_->print("VOC");

  // ── VOC value ───────────────────────────────────────────────────────
  display_->setTextSize(3);
  display_->setCursor(65, 84);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.voc_index, 3, 0, buf);
    char *p = buf;
    while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("---");
  }

  // ── NOX label ───────────────────────────────────────────────────────
  display_->setTextSize(2);
  display_->setCursor(5, 109);
  display_->print("NOX");

  // ── NOX value ───────────────────────────────────────────────────────
  display_->setTextSize(3);
  display_->setCursor(65, 108);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.nox_index, 3, 0, buf);
    char *p = buf;
    while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("---");
  }

  // ── Divider line ────────────────────────────────────────────────────
  display_->fillRect(5, 119, 190, 3, GxEPD_WHITE);

  // ── PM Labels ───────────────────────────────────────────────────────
  display_->setTextSize(2);

  display_->setCursor(5, 136);
  display_->print("PM");
  display_->setCursor(30, 136);
  display_->print("1.0");

  display_->setCursor(5, 150);
  display_->print("PM");
  display_->setCursor(30, 150);
  display_->print("2.5");

  display_->setCursor(5, 164);
  display_->print("PM");
  display_->setCursor(30, 164);
  display_->print("4.0");

  display_->setCursor(5, 177);
  display_->print("PM");
  display_->setCursor(30, 177);
  display_->print("10");

  // ── PM Values ───────────────────────────────────────────────────────
  display_->setCursor(66, 136);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.pm1p0, 4, 1, buf);
    char *p = buf; while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }

  display_->setCursor(66, 150);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.pm2p5, 4, 1, buf);
    char *p = buf; while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }

  display_->setCursor(66, 164);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.pm4p0, 4, 1, buf);
    char *p = buf; while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }

  display_->setCursor(66, 177);
  if (sensor_data_.valid) {
    dtostrf(sensor_data_.pm10p0, 4, 1, buf);
    char *p = buf; while (*p == ' ') p++;
    display_->print(p);
  } else {
    display_->print("--.-");
  }

  // ── Face bitmap (good vs bad air quality) ─────────────────────────
  bool good_air = sensor_data_.valid
      && sensor_data_.co2 < AQ_CO2_MAX
      && sensor_data_.voc_index < AQ_VOC_MAX
      && sensor_data_.nox_index <= AQ_NOX_MAX
      && sensor_data_.pm1p0 < AQ_PM_MAX
      && sensor_data_.pm2p5 < AQ_PM_MAX
      && sensor_data_.pm4p0 < AQ_PM_MAX
      && sensor_data_.pm10p0 < AQ_PM_MAX;

  if (good_air) {
    display_->drawBitmap(126, 141, image_happy_face_aq_bits, 58, 28, GxEPD_WHITE);
  } else {
    display_->drawBitmap(126, 143, image_sad_face_aq_bits, 58, 28, GxEPD_WHITE);
  }

  // ── Last update label ───────────────────────────────────────────────
  display_->setTextSize(1);
  display_->setCursor(97, 189);
  display_->print("LAST UPDATE");

  // ── Last update value ───────────────────────────────────────────────
  display_->setCursor(169, 189);
  if (sensor_data_.valid) {
    unsigned long elapsed = millis() - last_update_millis_;
    char elapsed_str[24];
    format_elapsed_(elapsed, elapsed_str, sizeof(elapsed_str));
    display_->print(elapsed_str);
  } else {
    display_->print("--:--");
  }

  display_->display(true);  // partial refresh
}

// ─── Setup ──────────────────────────────────────────────────────────────────

void AQAIOComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AQAIO (SEN66 + e-paper)...");

  // Initialize I2C for SEN66
  Wire.begin(this->i2c_sda_, this->i2c_scl_);

  // Initialize SPI and display
  SPI.begin(this->epd_sclk_, -1, this->epd_mosi_, this->epd_cs_);
  this->display_ = new GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT>(
      GxEPD2_154_D67(this->epd_cs_, this->epd_dc_, this->epd_rst_, this->epd_busy_));
  this->display_->init(0);
  this->display_->setRotation(this->display_rotation_);
  this->display_->setFullWindow();

  // Show splash screen
  this->draw_splash_();

  // Initialize SEN66
  this->sen66_.begin(Wire, SEN66_I2C_ADDR_6B);

  char error_message[64];
  int16_t error;

  error = this->sen66_.deviceReset();
  if (error != 0) {
    errorToString(error, error_message, sizeof(error_message));
    ESP_LOGE(TAG, "SEN66 deviceReset() error: %s", error_message);
  }
  // Feed watchdog during long delay (WDT timeout is ~5s)
  for (int i = 0; i < 12; i++) {
    delay(100);
    App.feed_wdt();
  }

  // Read serial number
  int8_t serial_number[32] = {0};
  error = this->sen66_.getSerialNumber(serial_number, 32);
  if (error != 0) {
    errorToString(error, error_message, sizeof(error_message));
    ESP_LOGW(TAG, "SEN66 getSerialNumber() error: %s", error_message);
  } else {
    ESP_LOGI(TAG, "SEN66 Serial: %s", (const char *) serial_number);
  }

  // Start continuous measurement
  error = this->sen66_.startContinuousMeasurement();
  if (error != 0) {
    errorToString(error, error_message, sizeof(error_message));
    ESP_LOGE(TAG, "SEN66 startContinuousMeasurement() error: %s", error_message);
  } else {
    this->sensor_ready_ = true;
    ESP_LOGI(TAG, "SEN66 continuous measurement started.");
  }

  // Discard first 10 readings (initial values are inaccurate)
  if (this->sensor_ready_) {
    ESP_LOGI(TAG, "Warming up sensor (10 readings)...");
    float _pm1, _pm2, _pm4, _pm10, _hum, _temp, _voc, _nox;
    uint16_t _co2;
    for (int i = 0; i < 10; i++) {
      // Feed watchdog during each 1s warm-up delay
      for (int j = 0; j < 10; j++) {
        delay(100);
        App.feed_wdt();
      }
      this->sen66_.readMeasuredValues(_pm1, _pm2, _pm4, _pm10,
                                      _hum, _temp, _voc, _nox, _co2);
      ESP_LOGD(TAG, "  warmup %d/10", i + 1);
    }
    ESP_LOGI(TAG, "Sensor warm-up complete.");
  }

  // Draw initial display with placeholder data
  this->draw_display_();
}

// ─── Update (called every update_interval) ──────────────────────────────────

void AQAIOComponent::update() {
  if (!this->sensor_ready_) {
    // Retry starting measurement
    int16_t error = this->sen66_.startContinuousMeasurement();
    if (error == 0) {
      this->sensor_ready_ = true;
      ESP_LOGI(TAG, "SEN66 measurement started (retry).");
    }
    return;
  }

  // Read sensor data
  float pm1p0 = 0, pm2p5 = 0, pm4p0 = 0, pm10p0 = 0;
  float humidity = 0, temperature = 0;
  float voc_index = 0, nox_index = 0;
  uint16_t co2 = 0;

  char error_message[64];
  int16_t error = this->sen66_.readMeasuredValues(
      pm1p0, pm2p5, pm4p0, pm10p0,
      humidity, temperature,
      voc_index, nox_index, co2);

  if (error != 0) {
    errorToString(error, error_message, sizeof(error_message));
    ESP_LOGW(TAG, "readMeasuredValues() error: %s", error_message);
    // Keep showing last valid data if available
  } else {
    this->sensor_data_.pm1p0 = pm1p0;
    this->sensor_data_.pm2p5 = pm2p5;
    this->sensor_data_.pm4p0 = pm4p0;
    this->sensor_data_.pm10p0 = pm10p0;
    this->sensor_data_.humidity = humidity;
    this->sensor_data_.temperature = temperature;
    this->sensor_data_.voc_index = voc_index;
    this->sensor_data_.nox_index = nox_index;
    this->sensor_data_.co2 = co2;
    this->sensor_data_.valid = true;
    this->last_update_millis_ = millis();

    ESP_LOGD(TAG, "T=%.1f°C RH=%.1f%% CO2=%u VOC=%.0f NOx=%.0f "
                   "PM1=%.1f PM2.5=%.1f PM4=%.1f PM10=%.1f",
             temperature, humidity, co2, voc_index, nox_index,
             pm1p0, pm2p5, pm4p0, pm10p0);

    // Publish to Home Assistant sensors
    if (this->temperature_sensor_ != nullptr)
      this->temperature_sensor_->publish_state(temperature);
    if (this->humidity_sensor_ != nullptr)
      this->humidity_sensor_->publish_state(humidity);
    if (this->co2_sensor_ != nullptr)
      this->co2_sensor_->publish_state(static_cast<float>(co2));
    if (this->voc_sensor_ != nullptr)
      this->voc_sensor_->publish_state(voc_index);
    if (this->nox_sensor_ != nullptr)
      this->nox_sensor_->publish_state(nox_index);
    if (this->pm_1_0_sensor_ != nullptr)
      this->pm_1_0_sensor_->publish_state(pm1p0);
    if (this->pm_2_5_sensor_ != nullptr)
      this->pm_2_5_sensor_->publish_state(pm2p5);
    if (this->pm_4_0_sensor_ != nullptr)
      this->pm_4_0_sensor_->publish_state(pm4p0);
    if (this->pm_10_0_sensor_ != nullptr)
      this->pm_10_0_sensor_->publish_state(pm10p0);
  }

  // Update e-paper display
  this->draw_display_();
}

// ─── Dump Config ────────────────────────────────────────────────────────────

void AQAIOComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AQAIO (SEN66 + e-paper):");
  ESP_LOGCONFIG(TAG, "  I2C SDA: %d, SCL: %d", this->i2c_sda_, this->i2c_scl_);
  ESP_LOGCONFIG(TAG, "  EPD MOSI: %d, SCLK: %d, CS: %d, DC: %d, RST: %d, BUSY: %d",
                this->epd_mosi_, this->epd_sclk_, this->epd_cs_, this->epd_dc_,
                this->epd_rst_, this->epd_busy_);
  ESP_LOGCONFIG(TAG, "  Display rotation: %d", this->display_rotation_);
  ESP_LOGCONFIG(TAG, "  Temperature unit: %s", this->use_fahrenheit_ ? "°F" : "°C");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
  LOG_SENSOR("  ", "CO2", this->co2_sensor_);
  LOG_SENSOR("  ", "VOC Index", this->voc_sensor_);
  LOG_SENSOR("  ", "NOx Index", this->nox_sensor_);
  LOG_SENSOR("  ", "PM 1.0", this->pm_1_0_sensor_);
  LOG_SENSOR("  ", "PM 2.5", this->pm_2_5_sensor_);
  LOG_SENSOR("  ", "PM 4.0", this->pm_4_0_sensor_);
  LOG_SENSOR("  ", "PM 10", this->pm_10_0_sensor_);
}

}  // namespace aqaio
}  // namespace esphome
