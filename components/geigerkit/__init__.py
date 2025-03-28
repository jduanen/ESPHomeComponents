import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    ICON_RADIOACTIVE,
    STATE_CLASS_MEASUREMENT
)

CODEOWNERS = ["@jduanen"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = []
MULTI_CONF = False

CPM_CONF = "cpm"
SIEVERTS_CONF = "uSv_hr"
VOLTS_CONF = "volts"

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
            cv.Optional(CPM_CONF): sensor.sensor_schema(
                icon=ICON_RADIOACTIVE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(SIEVERTS_CONF): sensor.sensor_schema(
                icon=ICON_RADIOACTIVE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(VOLTS_CONF): sensor.sensor_schema(
                icon=ICON_RADIOACTIVE,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

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

    if cpm_config := config.get(CPM_CONF):
        sens = await sensor.new_sensor(cpm_config)
        cg.add(var.set_cpm_sensor(sens))
    if usv_config := config.get(SIEVERTS_CONF):
        sens = await sensor.new_sensor(usv_config)
        cg.add(var.set_sieverts_sensor(sens))
    if v_config := config.get(VOLTS_CONF):
        sens = await sensor.new_sensor(v_config)
        cg.add(var.set_volts_sensor(sens))
