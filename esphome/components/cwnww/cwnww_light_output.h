#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/light/light_output.h"

namespace esphome {
namespace cwnww {

class CWNWWLightOutput : public light::LightOutput {
 public:
  void set_cold_white(output::FloatOutput *cold_white) { cold_white_ = cold_white; }
  void set_neutral_white(output::FloatOutput *neutral_white) { neutral_white_ = neutral_white; }
  void set_warm_white(output::FloatOutput *warm_white) { warm_white_ = warm_white; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
    return traits;
  }

  void write_state(light::LightState *state) override {
      if (!state->current_values.is_on()) {
          this->cold_white_->set_level(0.0f);
          this->neutral_white_->set_level(0.0f);
          this->warm_white_->set_level(0.0f);
          return;
      }
  
      float kelvin = state->current_values.get_color_temperature();
      float brightness = state->current_values.get_brightness();
  
      // Convert Kelvin to Mireds if required
      float mireds = 1000000.0f / kelvin;
  
      ESP_LOGI("cwnww", "Kelvin: %f, Mireds: %f, Brightness: %f", kelvin, mireds, brightness);
  
      // Define fixed temperature values
      const float cold_white_temperature = 3500.0f;
      const float neutral_white_temperature = 1800.0f;
      const float warm_white_temperature = 1000.0f;
  
      float cwhite = 0.0f, nwhite = 0.0f, wwhite = 0.0f;
  
      if (kelvin >= cold_white_temperature) {
          cwhite = brightness;
      } else if (kelvin <= warm_white_temperature) {
          wwhite = brightness;
      } else {
          float blend = (cold_white_temperature - kelvin) /
                        (cold_white_temperature - warm_white_temperature);
          cwhite = brightness * blend;
          wwhite = brightness * (1.0f - blend);
          nwhite = brightness * (1.0f - cwhite - wwhite);
      }
  
      ESP_LOGI("cwnww", "Levels -> Cold White: %f, Neutral White: %f, Warm White: %f", cwhite, nwhite, wwhite);
  
      this->cold_white_->set_level(cwhite);
      this->neutral_white_->set_level(nwhite);
      this->warm_white_->set_level(wwhite);
  }

 protected:
  output::FloatOutput *cold_white_;
  output::FloatOutput *neutral_white_;
  output::FloatOutput *warm_white_;
};

}  // namespace cwnww
}  // namespace esphome
