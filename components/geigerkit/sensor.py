import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import uart
from esphome.const import (
    CONF_ID,
    ICON_RADIOACTIVE,
    STATE_CLASS_MEASUREMENT
)

CODEOWNERS = ["@jduanen"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = []
MULTI_CONF = False

ns = cg.esphome_ns.namespace("geigerkit_ns")
GeigerKitComponent = ns.class_(
    "GeigerKitComponent",
    uart.UARTDevice,
    cg.Component
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

# code generation
async def to_code(config):
    # declare new component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # configure the component
    #### TODO
