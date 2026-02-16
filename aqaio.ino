/*
 * AQAIO - Air Quality All-In-One
 * ESP32-C3 + SEN66 + GDEH0154D67 (1.54" e-paper)
 *
 * Reads all SEN66 environmental sensor data over I2C and
 * displays readings on a 200x200 B&W e-paper display.
 *
 * Wiring:
 * | Function | ESP32-C3 Pin | Component         |
 * |----------|:------------:|-------------------|
 * | I2C SDA  |      10      | SEN66             |
 * | I2C SCL  |       9      | SEN66             |
 * | SPI MOSI |       7      | EPD (GDEH0154D67) |
 * | SPI SCLK |       6      | EPD (GDEH0154D67) |
 * | CS       |       5      | EPD (GDEH0154D67) |
 * | DC       |       4      | EPD (GDEH0154D67) |
 * | RST      |       3      | EPD (GDEH0154D67) |
 * | BUSY     |       2      | EPD (GDEH0154D67) |
 */

#include <Wire.h>
#include <SPI.h>
#include <SensirionI2cSen66.h>
#include <GxEPD2_BW.h>
#include <Fonts/Org_01.h>

// ─── User Configuration ─────────────────────────────────────────────────────

// Display rotation (0 = vertical, 3 = horizontal)
#define DISPLAY_VERTICAL_UP 0
#define DISPLAY_VERTICAL_DOWN 2
#define DISPLAY_HORIZONTAL_LEFT 1
#define DISPLAY_HORIZONTAL_RIGHT 3
#define DISPLAY_ROTATION DISPLAY_VERTICAL_UP

// Update interval in seconds (how often to refresh the display)
#define UPDATE_INTERVAL_S 60

// Temperature unit: set to true for °F, false for °C
#define USE_FAHRENHEIT false

// Air quality thresholds for "good" indicator
#define AQ_CO2_MAX      1000
#define AQ_VOC_MAX      150
#define AQ_NOX_MAX      1
#define AQ_PM_MAX       12

// ─── Pin Definitions ────────────────────────────────────────────────────────

// I2C pins (SEN66)
#define I2C_SDA 10
#define I2C_SCL 9

// SPI pins (e-paper)
#define EPD_MOSI 7
#define EPD_MISO -1
#define EPD_SCLK 6
#define EPD_CS   5
#define EPD_DC   4
#define EPD_RST  3
#define EPD_BUSY 2

// ─── Bitmaps (from layout) ──────────────────────────────────────────────────

static const unsigned char PROGMEM image_sad_face_aq_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x0f,0x00,0x00,0x00,0x3c,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x3f,0x00,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xf0,0x00,0x00,0x00,0x03,0xff,0x00,0x3f,0xf0,0x00,0x00,0x00,0x03,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xf0,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x0f,0x00,0x3c,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

static const unsigned char PROGMEM image_happy_face_aq_bits[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x3f,0xcc,0x00,0x00,0x00,0x0f,0xf3,0x00,0x3f,0xcc,0x00,0x00,0x00,0x0f,0xf3,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x3f,0xfc,0x00,0x00,0x00,0x0f,0xff,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x0f,0xf0,0x00,0x00,0x00,0x03,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x00};

static const unsigned char PROGMEM image_weather_humidity_bits[] = {0x04,0x00,0x04,0x00,0x0c,0x00,0x0e,0x00,0x1e,0x00,0x1f,0x00,0x3f,0x80,0x3f,0x80,0x7e,0xc0,0x7f,0x40,0xff,0x60,0xff,0xe0,0x7f,0xc0,0x7f,0xc0,0x3f,0x80,0x0f,0x00};

static const unsigned char PROGMEM image_weather_temperature_bits[] = {0x1c,0x00,0x22,0x02,0x2b,0x05,0x2a,0x02,0x2b,0x38,0x2a,0x60,0x2b,0x40,0x2a,0x40,0x2a,0x60,0x49,0x38,0x9c,0x80,0xae,0x80,0xbe,0x80,0x9c,0x80,0x41,0x00,0x3e,0x00};

// ─── Globals ────────────────────────────────────────────────────────────────

// Ensure NO_ERROR is defined
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

// Display: GxEPD2_154_D67 is 200x200, using full frame buffer
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT>
    display(GxEPD2_154_D67(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// SEN66 sensor
SensirionI2cSen66 sensor;

// Sensor data
struct SensorData {
    float pm1p0;
    float pm2p5;
    float pm4p0;
    float pm10p0;
    float humidity;
    float temperature;
    float vocIndex;
    float noxIndex;
    uint16_t co2;
    bool valid;
};

static SensorData sensorData = {0, 0, 0, 0, 0, 0, 0, 0, 0, false};
static unsigned long lastUpdateMillis = 0;
static bool sensorReady = false;

// ─── Helper Functions ───────────────────────────────────────────────────────

float toDisplayTemp(float tempC) {
    if (USE_FAHRENHEIT) {
        return tempC * 9.0f / 5.0f + 32.0f;
    }
    return tempC;
}

const char* tempUnitStr() {
    return USE_FAHRENHEIT ? "F" : "C";
}

/**
 * Formats elapsed time since last update as a human-readable string.
 */
void formatElapsed(unsigned long elapsedMs, char* buf, size_t bufLen) {
    unsigned long secs = elapsedMs / 1000;
    if (secs < 60) {
        snprintf(buf, bufLen, "%lus ago", secs);
    } else if (secs < 3600) {
        unsigned long mins = secs / 60;
        snprintf(buf, bufLen, "%lum ago", mins);
    } else {
        unsigned long hrs = secs / 3600;
        unsigned long mins = (secs % 3600) / 60;
        snprintf(buf, bufLen, "%luh%lum ago", hrs, mins);
    }
}

// ─── Display Rendering ─────────────────────────────────────────────────────

void drawDisplay() {
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setTextWrap(false);
    display.setFont(&Org_01);

    char buf[16];

    // ── Temperature value ───────────────────────────────────────────────
    display.setTextSize(3);
    display.setCursor(22, 24);
    if (sensorData.valid) {
        float dispTemp = toDisplayTemp(sensorData.temperature);
        dtostrf(dispTemp, 4, 1, buf);
        char* p = buf;
        while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }
    display.setTextSize(2);
    display.print(tempUnitStr());

    // ── Temperature icon ────────────────────────────────────────────────
    display.drawBitmap(5, 11, image_weather_temperature_bits, 16, 16, GxEPD_WHITE);

    // ── Humidity icon ───────────────────────────────────────────────────
    display.drawBitmap(111, 11, image_weather_humidity_bits, 11, 16, GxEPD_WHITE);

    // ── Humidity value ──────────────────────────────────────────────────
    display.setTextSize(3);
    display.setCursor(125, 24);
    if (sensorData.valid) {
        dtostrf(sensorData.humidity, 4, 1, buf);
        char* p = buf;
        while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }

    // ── Humidity unit ───────────────────────────────────────────────────
    display.setTextSize(2);
    display.setCursor(185, 25);
    display.print("%");

    // ── Divider line ────────────────────────────────────────────────────
    display.fillRect(5, 32, 190, 3, GxEPD_WHITE);

    // ── CO2 label ───────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setCursor(5, 61);
    display.print("CO2");

    // ── CO2 value ───────────────────────────────────────────────────────
    display.setTextSize(4);
    display.setCursor(65, 59);
    if (sensorData.valid) {
        display.print(sensorData.co2);
    } else {
        display.print("----");
    }

    // ── CO2 unit ────────────────────────────────────────────────────────
    int16_t ppmX = display.getCursorX() + 5;
    display.setTextSize(2);
    display.setCursor(ppmX, 61);
    display.print("PPM");

    // ── VOC label ───────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setCursor(5, 85);
    display.print("VOC");

    // ── VOC value ───────────────────────────────────────────────────────
    display.setTextSize(3);
    display.setCursor(65, 84);
    if (sensorData.valid) {
        dtostrf(sensorData.vocIndex, 3, 0, buf);
        char* p = buf;
        while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("---");
    }

    // ── NOX label ───────────────────────────────────────────────────────
    display.setTextSize(2);
    display.setCursor(5, 109);
    display.print("NOX");

    // ── NOX value ───────────────────────────────────────────────────────
    display.setTextSize(3);
    display.setCursor(65, 108);
    if (sensorData.valid) {
        dtostrf(sensorData.noxIndex, 3, 0, buf);
        char* p = buf;
        while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("---");
    }

    // ── Divider line ────────────────────────────────────────────────────
    display.fillRect(5, 119, 190, 3, GxEPD_WHITE);

    // ── PM Labels ───────────────────────────────────────────────────────
    display.setTextSize(2);

    display.setCursor(5, 136);
    display.print("PM");
    display.setCursor(30, 136);
    display.print("1.0");

    display.setCursor(5, 150);
    display.print("PM");
    display.setCursor(30, 150);
    display.print("2.5");

    display.setCursor(5, 164);
    display.print("PM");
    display.setCursor(30, 164);
    display.print("4.0");

    display.setCursor(5, 177);
    display.print("PM");
    display.setCursor(30, 177);
    display.print("10");

    // ── PM Values ───────────────────────────────────────────────────────
    display.setCursor(66, 136);
    if (sensorData.valid) {
        dtostrf(sensorData.pm1p0, 4, 1, buf);
        char* p = buf; while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }

    display.setCursor(66, 150);
    if (sensorData.valid) {
        dtostrf(sensorData.pm2p5, 4, 1, buf);
        char* p = buf; while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }

    display.setCursor(66, 164);
    if (sensorData.valid) {
        dtostrf(sensorData.pm4p0, 4, 1, buf);
        char* p = buf; while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }

    display.setCursor(66, 177);
    if (sensorData.valid) {
        dtostrf(sensorData.pm10p0, 4, 1, buf);
        char* p = buf; while (*p == ' ') p++;
        display.print(p);
    } else {
        display.print("--.-");
    }

    // ── Face bitmap (good vs bad air quality) ─────────────────────────
    bool goodAir = sensorData.valid
        && sensorData.co2 < AQ_CO2_MAX
        && sensorData.vocIndex < AQ_VOC_MAX
        && sensorData.noxIndex <= AQ_NOX_MAX
        && sensorData.pm1p0 < AQ_PM_MAX
        && sensorData.pm2p5 < AQ_PM_MAX
        && sensorData.pm4p0 < AQ_PM_MAX
        && sensorData.pm10p0 < AQ_PM_MAX;

    if (goodAir) {
        display.drawBitmap(126, 141, image_happy_face_aq_bits, 58, 28, GxEPD_WHITE);
    } else {
        display.drawBitmap(126, 143, image_sad_face_aq_bits, 58, 28, GxEPD_WHITE);
    }

    // ── Last update label ───────────────────────────────────────────────
    display.setTextSize(1);
    display.setCursor(97, 189);
    display.print("LAST UPDATE");

    // ── Last update value ───────────────────────────────────────────────
    display.setCursor(169, 189);
    if (sensorData.valid) {
        unsigned long elapsed = millis() - lastUpdateMillis;
        char elapsedStr[24];
        formatElapsed(elapsed, elapsedStr, sizeof(elapsedStr));
        display.print(elapsedStr);
    } else {
        display.print("--:--");
    }

    display.display(true); // partial refresh
}

void drawSplash() {
    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);
    display.setFont(&Org_01);
    display.setTextSize(4);
    display.setCursor(30, 90);
    display.print("AQAIO");
    display.setTextSize(2);
    display.setCursor(20, 120);
    display.print("Initializing...");
    display.display(false);
}

// ─── Setup ──────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500); // Allow USB CDC to connect on ESP32-C3

    Serial.println("AQAIO - Air Quality All-In-One");
    Serial.println("Initializing...");

    // Initialize I2C for SEN66
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize SPI and display
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI, EPD_CS);
    display.init(0); // 0 = no diagnostic serial output from GxEPD2
    display.setRotation(DISPLAY_ROTATION);
    display.setFullWindow();

    // Show splash screen
    drawSplash();

    // Initialize SEN66
    sensor.begin(Wire, SEN66_I2C_ADDR_6B);

    char errorMessage[64];
    int16_t error;

    error = sensor.deviceReset();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.print("SEN66 deviceReset() error: ");
        Serial.println(errorMessage);
    }
    delay(1200); // required after reset

    // Print serial number
    int8_t serialNumber[32] = {0};
    error = sensor.getSerialNumber(serialNumber, 32);
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.print("SEN66 getSerialNumber() error: ");
        Serial.println(errorMessage);
    } else {
        Serial.print("SEN66 Serial: ");
        Serial.println((const char*)serialNumber);
    }

    // Start continuous measurement
    error = sensor.startContinuousMeasurement();
    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.print("SEN66 startContinuousMeasurement() error: ");
        Serial.println(errorMessage);
    } else {
        sensorReady = true;
        Serial.println("SEN66 continuous measurement started.");
    }

    Serial.print("Update interval: ");
    Serial.print(UPDATE_INTERVAL_S);
    Serial.println("s");

    // Discard first 10 readings (initial values are inaccurate)
    if (sensorReady) {
        Serial.println("Warming up sensor (10 readings)...");
        float _pm1, _pm2, _pm4, _pm10, _hum, _temp, _voc, _nox;
        uint16_t _co2;
        for (int i = 0; i < 10; i++) {
            delay(1000);
            sensor.readMeasuredValues(_pm1, _pm2, _pm4, _pm10,
                                     _hum, _temp, _voc, _nox, _co2);
            Serial.printf("  warmup %d/10\n", i + 1);
        }
        Serial.println("Sensor warm-up complete.");
    }

    // Draw initial display with placeholder data
    drawDisplay();
}

// ─── Loop ───────────────────────────────────────────────────────────────────

void loop() {
    if (!sensorReady) {
        // Sensor failed to start, retry
        delay(5000);
        char errorMessage[64];
        int16_t error = sensor.startContinuousMeasurement();
        if (error == NO_ERROR) {
            sensorReady = true;
            Serial.println("SEN66 measurement started (retry).");
        }
        return;
    }

    // Read sensor data
    float pm1p0 = 0, pm2p5 = 0, pm4p0 = 0, pm10p0 = 0;
    float humidity = 0, temperature = 0;
    float vocIndex = 0, noxIndex = 0;
    uint16_t co2 = 0;

    char errorMessage[64];
    int16_t error = sensor.readMeasuredValues(
        pm1p0, pm2p5, pm4p0, pm10p0,
        humidity, temperature,
        vocIndex, noxIndex, co2);

    if (error != NO_ERROR) {
        errorToString(error, errorMessage, sizeof(errorMessage));
        Serial.print("readMeasuredValues() error: ");
        Serial.println(errorMessage);
        // Keep showing last valid data if available
    } else {
        sensorData.pm1p0 = pm1p0;
        sensorData.pm2p5 = pm2p5;
        sensorData.pm4p0 = pm4p0;
        sensorData.pm10p0 = pm10p0;
        sensorData.humidity = humidity;
        sensorData.temperature = temperature;
        sensorData.vocIndex = vocIndex;
        sensorData.noxIndex = noxIndex;
        sensorData.co2 = co2;
        sensorData.valid = true;
        lastUpdateMillis = millis();

        // Debug output
        Serial.printf("T=%.1f°C  RH=%.1f%%  CO2=%u  VOC=%.0f  NOx=%.0f  "
                       "PM1=%.1f  PM2.5=%.1f  PM4=%.1f  PM10=%.1f\n",
                       temperature, humidity, co2, vocIndex, noxIndex,
                       pm1p0, pm2p5, pm4p0, pm10p0);
    }

    // Update display
    drawDisplay();

    // Wait for next update
    delay((unsigned long)UPDATE_INTERVAL_S * 1000UL);
}
