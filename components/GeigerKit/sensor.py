import esphome.codegen as cg
#import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import ICON_RADIOACTIVE

CONF_MY_CUSTOM_ID = "GeigerKit_id"

CODEOWNERS = ["@jduanen"]
DEPENDENCIES = ["uart"]

geigerkit_sensor_ns = cg.esphome_ns.namespace("GeigerKit_sensor")

GeigerKitSensor = geigerkit_sensor_ns.class_(
    "GeigerKitSensor",
    cg.Component,
    sensor.Sensor,
    uart.UARTDevice
)

CONFIG_SCHEMA = sensor.sensor_schema(
    sensor.sensor_schema(
        GeigerKitSensor,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=1
    )
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
