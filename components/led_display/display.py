import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_STATE,
)


CODEOWNERS = ["@jduanen"]
AUTO_LOAD = ["display"]
DEPENDENCIES = ["display"]

CONF_SCROLL_MODE = "scroll_mode"
CONF_SCROLL_ENABLE = "scroll_enable"
CONF_SCROLL_SPEED = "scroll_speed"
CONF_SCROLL_DELAY = "scroll_delay"
CONF_SCROLL_DWELL = "scroll_dwell"


led_display_ns = cg.esphome_ns.namespace('led_display')

ScrollMode = led_display_ns.enum("ScrollMode")
SCROLL_MODES = {
    "CONTINUOUS": ScrollMode.CONTINUOUS,
    "STOP": ScrollMode.STOP,
}

LedDisplayComponent = led_display_ns.class_(
    "LedDisplayComponent", display.DisplayBuffer, cg.PollingComponent
)
LedDisplayComponentRef = LedDisplayComponent.operator("ref")

CONFIG_SCHEMA = (
    display.BASIC_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(LedDisplayComponent),
            cv.Optional(CONF_SCROLL_MODE, default="CONTINUOUS"): cv.enum(
                SCROLL_MODES, upper=True
            ),
            cv.Optional(CONF_SCROLL_ENABLE, default=False): cv.boolean,  #### FIXME
            cv.Optional(
                CONF_SCROLL_SPEED, default="250ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_SCROLL_DELAY, default="1000ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_SCROLL_DWELL, default="1000ms"
            ): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("500ms"))
)


####cv.Optional(CONF_BRIGHTNESS, default=50): cv.int_range(0, 100, min_included=True, max_included=True),

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    cg.add(var.set_scroll_mode(config[CONF_SCROLL_MODE]))
    cg.add(var.set_scroll(config[CONF_SCROLL_ENABLE]))
    cg.add(var.set_scroll_speed(config[CONF_SCROLL_SPEED]))
    cg.add(var.set_scroll_dwell(config[CONF_SCROLL_DWELL]))
    cg.add(var.set_scroll_delay(config[CONF_SCROLL_DELAY]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(LedDisplayComponentRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

