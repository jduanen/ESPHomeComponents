import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID


CONF_MY_CUSTOM_ID = "GeigerKit_id"

CODEOWNERS = ["@jduanen"]
DEPENDENCIES = []
AUTO_LOAD = []
MULTI_CONF = False

ns = cg.esphome_ns.namespace("geigerkit_ns")
GeigerKit = ns.class_("geigerkit", cg.Component)

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(GeigerKit),
        # schema defininition, including options for the component
    }
)

# code generation
async def to_code(config):
    # declare new component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # configure the component
    #### TODO
