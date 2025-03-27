import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    ICON_RADIOACTIVE,
    STATE_CLASS_MEASUREMENT
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = []
MULTI_CONF = False

ns = cg.esphome_ns.namespace("geigerkit_ns")
GeigerKitSensor = ns.class_(
    "GeigerKitSensor",
    uart.UARTDevice,
    cg.Component
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(GeigerKitSensor),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

'''
CONFIG_SCHEMA = sensor.sensor_schema(
    sensor.sensor_schema(
        GeigerKitSensor,
        icon=ICON_RADIOACTIVE,
        accuracy_decimals=1,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(uart.UART_DEVICE_SCHEMA)
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(GeigerKitComponent),
            # schema defininition, including options for the component
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)
'''

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "GeigerKitSensor",
    baud_rate=9600,
    require_tx=False,
    require_rx=True,
    data_bits=8,
    parity=None,
    stop_bits=1,
)

# code generation
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # configure the component
    #### TODO
