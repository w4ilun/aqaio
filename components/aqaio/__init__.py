import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, time as time_
from esphome.const import CONF_ID, CONF_TIME_ID

CODEOWNERS = []

aqaio_ns = cg.esphome_ns.namespace("aqaio")
AQAIOComponent = aqaio_ns.class_("AQAIOComponent", cg.PollingComponent)

CONF_I2C_SDA = "i2c_sda"
CONF_I2C_SCL = "i2c_scl"
CONF_EPD_MOSI = "epd_mosi"
CONF_EPD_SCLK = "epd_sclk"
CONF_EPD_CS = "epd_cs"
CONF_EPD_DC = "epd_dc"
CONF_EPD_RST = "epd_rst"
CONF_EPD_BUSY = "epd_busy"
CONF_DISPLAY_ROTATION = "display_rotation"
CONF_USE_FAHRENHEIT = "use_fahrenheit"
CONF_AQ_CO2_MAX = "aq_co2_max"
CONF_AQ_VOC_MAX = "aq_voc_max"
CONF_AQ_NOX_MAX = "aq_nox_max"
CONF_AQ_PM_MAX = "aq_pm_max"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AQAIOComponent),
            cv.Optional(CONF_I2C_SDA, default=10): cv.int_,
            cv.Optional(CONF_I2C_SCL, default=9): cv.int_,
            cv.Optional(CONF_EPD_MOSI, default=7): cv.int_,
            cv.Optional(CONF_EPD_SCLK, default=6): cv.int_,
            cv.Optional(CONF_EPD_CS, default=5): cv.int_,
            cv.Optional(CONF_EPD_DC, default=4): cv.int_,
            cv.Optional(CONF_EPD_RST, default=3): cv.int_,
            cv.Optional(CONF_EPD_BUSY, default=2): cv.int_,
            cv.Optional(CONF_DISPLAY_ROTATION, default=0): cv.int_range(0, 3),
            cv.Optional(CONF_USE_FAHRENHEIT, default=False): cv.boolean,
            cv.Optional(CONF_TIME_ID): cv.use_id(time_.RealTimeClock),
            cv.Optional(CONF_AQ_CO2_MAX, default=1000): cv.int_range(min=0),
            cv.Optional(CONF_AQ_VOC_MAX, default=150): cv.int_range(min=0),
            cv.Optional(CONF_AQ_NOX_MAX, default=1): cv.int_range(min=0),
            cv.Optional(CONF_AQ_PM_MAX, default=12): cv.int_range(min=0),
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_i2c_pins(config[CONF_I2C_SDA], config[CONF_I2C_SCL]))
    cg.add(
        var.set_epd_pins(
            config[CONF_EPD_MOSI],
            config[CONF_EPD_SCLK],
            config[CONF_EPD_CS],
            config[CONF_EPD_DC],
            config[CONF_EPD_RST],
            config[CONF_EPD_BUSY],
        )
    )
    cg.add(var.set_display_rotation(config[CONF_DISPLAY_ROTATION]))
    cg.add(var.set_use_fahrenheit(config[CONF_USE_FAHRENHEIT]))
    cg.add(var.set_aq_thresholds(
        config[CONF_AQ_CO2_MAX],
        config[CONF_AQ_VOC_MAX],
        config[CONF_AQ_NOX_MAX],
        config[CONF_AQ_PM_MAX],
    ))

    if time_id := config.get(CONF_TIME_ID):
        time_var = await cg.get_variable(time_id)
        cg.add(var.set_time(time_var))

    cg.add_library("Wire", None)
    cg.add_library("SPI", None)
    cg.add_library("Sensirion/Sensirion Core", None)
    cg.add_library("Sensirion/Sensirion I2C SEN66", None)
    cg.add_library("adafruit/Adafruit BusIO", "1.16.2")
    cg.add_library("adafruit/Adafruit GFX Library", "1.11.11")
    cg.add_library("zinggjm/GxEPD2", None)
