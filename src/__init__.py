import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.sensor as esp_sensor
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_UPDATE_INTERVAL,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_PERCENT,
    ICON_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
)
