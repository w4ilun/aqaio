#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/time/real_time_clock.h"

#include <Wire.h>
#include <SPI.h>
#include <SensirionI2cSen66.h>
#include <GxEPD2_BW.h>
#include <Fonts/Org_01.h>

namespace esphome {
namespace aqaio {

struct SensorData {
  float pm1p0 = 0;
  float pm2p5 = 0;
  float pm4p0 = 0;
  float pm10p0 = 0;
  float humidity = 0;
  float temperature = 0;
  float voc_index = 0;
  float nox_index = 0;
  uint16_t co2 = 0;
  bool valid = false;
};

class AQAIOComponent : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Pin configuration (called from generated code)
  void set_i2c_pins(int sda, int scl) { i2c_sda_ = sda; i2c_scl_ = scl; }
  void set_epd_pins(int mosi, int sclk, int cs, int dc, int rst, int busy) {
    epd_mosi_ = mosi; epd_sclk_ = sclk;
    epd_cs_ = cs; epd_dc_ = dc; epd_rst_ = rst; epd_busy_ = busy;
  }
  void set_display_rotation(int rot) { display_rotation_ = rot; }
  void set_use_fahrenheit(bool f) { use_fahrenheit_ = f; }
  void set_time(time::RealTimeClock *time) { time_ = time; }
  void set_aq_thresholds(uint16_t co2, uint16_t voc, uint16_t nox, uint16_t pm) {
    aq_co2_max_ = co2; aq_voc_max_ = voc; aq_nox_max_ = nox; aq_pm_max_ = pm;
  }

  // Home Assistant sensor setters
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_humidity_sensor(sensor::Sensor *s) { humidity_sensor_ = s; }
  void set_co2_sensor(sensor::Sensor *s) { co2_sensor_ = s; }
  void set_voc_sensor(sensor::Sensor *s) { voc_sensor_ = s; }
  void set_nox_sensor(sensor::Sensor *s) { nox_sensor_ = s; }
  void set_pm_1_0_sensor(sensor::Sensor *s) { pm_1_0_sensor_ = s; }
  void set_pm_2_5_sensor(sensor::Sensor *s) { pm_2_5_sensor_ = s; }
  void set_pm_4_0_sensor(sensor::Sensor *s) { pm_4_0_sensor_ = s; }
  void set_pm_10_0_sensor(sensor::Sensor *s) { pm_10_0_sensor_ = s; }

 protected:
  // Display and sensor rendering
  void draw_display_();
  void draw_splash_();
  float to_display_temp_(float temp_c);
  const char *temp_unit_str_();

  // Pin config
  int i2c_sda_ = 10;
  int i2c_scl_ = 9;
  int epd_mosi_ = 7;
  int epd_sclk_ = 6;
  int epd_cs_ = 5;
  int epd_dc_ = 4;
  int epd_rst_ = 3;
  int epd_busy_ = 2;

  // Settings
  int display_rotation_ = 0;
  bool use_fahrenheit_ = false;
  uint16_t aq_co2_max_ = 1000;
  uint16_t aq_voc_max_ = 150;
  uint16_t aq_nox_max_ = 1;
  uint16_t aq_pm_max_ = 12;

  // State
  SensorData sensor_data_{};
  unsigned long last_update_millis_ = 0;
  bool sensor_ready_ = false;
  char last_update_time_[9] = "--:--";
  unsigned long last_full_refresh_millis_ = 0;

  // Hardware objects (allocated in setup)
  SensirionI2cSen66 sen66_;
  GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> *display_ = nullptr;
  time::RealTimeClock *time_{nullptr};

  // Home Assistant sensors
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};
  sensor::Sensor *co2_sensor_{nullptr};
  sensor::Sensor *voc_sensor_{nullptr};
  sensor::Sensor *nox_sensor_{nullptr};
  sensor::Sensor *pm_1_0_sensor_{nullptr};
  sensor::Sensor *pm_2_5_sensor_{nullptr};
  sensor::Sensor *pm_4_0_sensor_{nullptr};
  sensor::Sensor *pm_10_0_sensor_{nullptr};
};

}  // namespace aqaio
}  // namespace esphome
