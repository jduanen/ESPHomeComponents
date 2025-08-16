from esphome import automation
import esphome.codegen as cg
from esphome.components import display
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_INTENSITY,
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

CONF_FLIP_X = "flip_x"
CONF_FONTS = "fonts"

from esphome.components import font

integration_ns = cg.esphome_ns.namespace("led_display")
ScrollMode = integration_ns.enum("ScrollMode")
SCROLL_MODES = {
    "CONTINUOUS": ScrollMode.CONTINUOUS,
    "STOP": ScrollMode.STOP,
}

led_display_ns = cg.esphome_ns.namespace('led_display')
LedDisplayComponent = led_display_ns.class_(
    "LedDisplayComponent", display.DisplayBuffer, cg.PollingComponent
)
LedDisplayComponentRef = LedDisplayComponent.operator("ref")

CONFIG_SCHEMA = (
    display.BASIC_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(LedDisplayComponent),
            cv.Required(CONF_FONTS): cv.ensure_list(cv.use_id(font.font_ns.class_('Font'))),
            cv.Optional(CONF_INTENSITY, default=50): cv.int_range(0, 100, min_included=True, max_included=True),
            cv.Optional(CONF_SCROLL_MODE, default="CONTINUOUS"): cv.enum(
                SCROLL_MODES, upper=True
            ),
            cv.Optional(CONF_SCROLL_ENABLE, default=True): cv.boolean,
            cv.Optional(
                CONF_SCROLL_SPEED, default="250ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_SCROLL_DELAY, default="1000ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(
                CONF_SCROLL_DWELL, default="1000ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_FLIP_X, default=False): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("500ms"))   #### TODO think about this
)

async def to_code(config):
    fonts = []
    for fontId in config[CONF_FONTS]:
        font = await cg.get_variable(fontId)
        fonts.append(font)
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    cg.add(var.set_fonts(fonts))
    cg.add(var.set_intensity(config[CONF_INTENSITY]))
    cg.add(var.set_scroll_mode(config[CONF_SCROLL_MODE]))
    cg.add(var.set_scroll(config[CONF_SCROLL_ENABLE]))
    cg.add(var.set_scroll_speed(config[CONF_SCROLL_SPEED]))
    cg.add(var.set_scroll_dwell(config[CONF_SCROLL_DWELL]))
    cg.add(var.set_scroll_delay(config[CONF_SCROLL_DELAY]))
    cg.add(var.set_flip_x(config[CONF_FLIP_X]))

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(LedDisplayComponentRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))


# Display Visibility
LED_DISPLAY_OFF_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.GenerateID(): cv.use_id(LedDisplayComponent),
        cv.Optional(CONF_STATE, default=False): False,
    }
)

LED_DISPLAY_ON_ACTION_SCHEMA = automation.maybe_simple_id(
    {
        cv.GenerateID(): cv.use_id(LedDisplayComponent),
        cv.Optional(CONF_STATE, default=True): True,
    }
)

DisplayVisibilityAction = led_display_ns.class_("DisplayVisibilityAction", automation.Action)

@automation.register_action(
    "led_display.turn_off", DisplayVisibilityAction, LED_DISPLAY_OFF_ACTION_SCHEMA
)
@automation.register_action(
    "led_display.turn_on", DisplayVisibilityAction, LED_DISPLAY_ON_ACTION_SCHEMA
)
async def led_display_visible_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_state(config[CONF_STATE]))
    return var

# Display Invert
DisplayInvertAction = led_display_ns.class_("DisplayInvertAction", automation.Action)

@automation.register_action(
    "led_display.invert_off", DisplayInvertAction, LED_DISPLAY_OFF_ACTION_SCHEMA
)
@automation.register_action(
    "led_display.invert_on", DisplayInvertAction, LED_DISPLAY_ON_ACTION_SCHEMA
)
async def led_display_invert_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_state(config[CONF_STATE]))
    return var

# Display Intensity
LED_DISPLAY_INTENSITY_SCHEMA = cv.maybe_simple_value(
    {
        cv.GenerateID(): cv.use_id(LedDisplayComponent),
        cv.Optional(CONF_INTENSITY, default=50): cv.templatable(
            cv.int_range(min=0, max=100, min_included=True, max_included=True)
        ),
    },
    key=CONF_INTENSITY,
)

DisplayIntensityAction = led_display_ns.class_("DisplayIntensityAction", automation.Action)

@automation.register_action(
    "led_display.intensity", DisplayIntensityAction, LED_DISPLAY_INTENSITY_SCHEMA
)
async def led_display_intensity_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    template_ = await cg.templatable(config[CONF_INTENSITY], args, cg.uint8)
    cg.add(var.set_state(template_))
    return var
