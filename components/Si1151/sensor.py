import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_ILLUMINANCE,
    ICON_LIGHTBULB,
    STATE_CLASS_MEASUREMENT,
    UNIT_LUX,
)

DEPENDENCIES = ["i2c"]

si1151_ns = cg.esphome_ns.namespace("Si1151")
Si1151Component = si1151_ns.class_("Si1151Component", cg.PollingComponent, i2c.I2CDevice)

CONF_IR_CHANNEL = "infrared_channel"
CONF_VISIBLE_CHANNEL = "visible_channel"
CONF_UV_CHANNEL = "ultraviolet_channel"

infrared_channel_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_LUX,
    accuracy_decimals=4,
    icon=ICON_LIGHTBULB,
    device_class=DEVICE_CLASS_ILLUMINANCE,
    state_class=STATE_CLASS_MEASUREMENT,
)
visible_channel_schema = sensor.sensor_schema(
    unit_of_measurement=UNIT_LUX,
    accuracy_decimals=4,
    icon=ICON_LIGHTBULB,
    device_class=DEVICE_CLASS_ILLUMINANCE,
    state_class=STATE_CLASS_MEASUREMENT,
)
ultraviolet_channel_schema = sensor.sensor_schema(
    unit_of_measurement="UV Index",
    accuracy_decimals=2,
    icon=ICON_LIGHTBULB,
    device_class=DEVICE_CLASS_ILLUMINANCE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Si1151Component),
            cv.Optional(CONF_IR_CHANNEL): infrared_channel_schema,
            cv.Optional(CONF_VISIBLE_CHANNEL): visible_channel_schema,
            cv.Optional(CONF_UV_CHANNEL): ultraviolet_channel_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x53))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_IR_CHANNEL in config:
        sens = await sensor.new_sensor(config[CONF_IR_CHANNEL])
        cg.add(var.set_ir_sensor(sens))

    if CONF_VISIBLE_CHANNEL in config:
        sens = await sensor.new_sensor(config[CONF_VISIBLE_CHANNEL])
        cg.add(var.set_visible_sensor(sens))

    if CONF_UV_CHANNEL in config:
        sens = await sensor.new_sensor(config[CONF_UV_CHANNEL])
        cg.add(var.set_uv_sensor(sens))
