import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_BRIGHTNESS,
)

CODEOWNERS = ["@jduanen"]
AUTO_LOAD = ["display"]
DEPENDENCIES = ["display"]

led_display_ns = cg.esphome_ns.namespace('led_display')
LedDisplayComponent = led_display_ns.class_(
    "LedDisplayComponent", display.DisplayBuffer
)

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(LedDisplayComponent),
    cv.Optional(CONF_BRIGHTNESS, default=50): cv.int_range(
        0, 100, min_included=True, max_included=True),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
'''
    var = cg.new_Pvariable(config[CONF_ID])
    await display.new_display(config)
----
    var = await display.new_display(config)
    cg.add(var)
    cg.add(var.brightness(config["brightness"]))
'''
 