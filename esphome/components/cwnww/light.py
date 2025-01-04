from esphome import automation
from esphome.components import light, output
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_COLD_WHITE,
    CONF_WARM_WHITE,
    CONF_CONSTANT_BRIGHTNESS,
    CONF_OUTPUT_ID,
)

# Define the missing constant
CONF_NEUTRAL_WHITE = "neutral_white"

cwnww_ns = cg.esphome_ns.namespace("cwnww")
CWNWWLightOutput = cwnww_ns.class_("CWNWWLightOutput", light.LightOutput)

CONFIG_SCHEMA = light.RGB_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CWNWWLightOutput),
        cv.Required(CONF_COLD_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_NEUTRAL_WHITE): cv.use_id(output.FloatOutput),
        cv.Required(CONF_WARM_WHITE): cv.use_id(output.FloatOutput),
        cv.Optional(CONF_CONSTANT_BRIGHTNESS, default=False): cv.boolean,
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)

    # Register cold white output
    cwhite = await cg.get_variable(config[CONF_COLD_WHITE])
    cg.add(var.set_cold_white(cwhite))

    # Register neutral white output
    nwhite = await cg.get_variable(config[CONF_NEUTRAL_WHITE])
    cg.add(var.set_neutral_white(nwhite))

    # Register warm white output
    wwhite = await cg.get_variable(config[CONF_WARM_WHITE])
    cg.add(var.set_warm_white(wwhite))

    # Set constant brightness if applicable
    cg.add(var.set_constant_brightness(config[CONF_CONSTANT_BRIGHTNESS]))
