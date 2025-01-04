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
  void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
  void set_neutral_white_temperature(float neutral_white_temperature) { neutral_white_temperature_ = neutral_white_temperature; }
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
  void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }

  light::LightTraits get_traits() override {
      auto traits = light::LightTraits();
      traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
      
      // Set mireds (converted from Kelvin)
      traits.set_min_mireds(this->cold_white_temperature_);
      traits.set_max_mireds(this->warm_white_temperature_);
      
      return traits;
  }

  void write_state(light::LightState *state) override {
      if (!state->current_values.is_on()) {
          // Turn off all channels
          this->cold_white_->set_level(0.0f);
          this->neutral_white_->set_level(0.0f);
          this->warm_white_->set_level(0.0f);
          return;
      }
  
      // Retrieve Kelvin and Brightness
      float mireds = state->current_values.get_color_temperature();
      float kelvin = 1000000.0f / mireds;  // Convert mireds to Kelvin
      float brightness = state->current_values.get_brightness();
  
      // Debug log for raw Kelvin
      ESP_LOGI("cwnww", "Raw Kelvin (converted from mireds): %f", kelvin);
  
      // Clamp Kelvin to the defined range
      if (kelvin > this->cold_white_temperature_) {
          kelvin = this->cold_white_temperature_;
      } else if (kelvin < this->warm_white_temperature_) {
          kelvin = this->warm_white_temperature_;
      }
  
      // Debug log for clamped Kelvin
      ESP_LOGI("cwnww", "Clamped Kelvin: %f", kelvin);
  
      // Calculate output levels
      float cwhite = 0.0f, nwhite = 0.0f, wwhite = 0.0f;
  
      if (kelvin >= this->cold_white_temperature_) {
          // Full cold white
          cwhite = brightness;
          ESP_LOGI("cwnww", "Full Cold White active.");
      } else if (kelvin <= this->warm_white_temperature_) {
          // Full warm white
          wwhite = brightness;
          ESP_LOGI("cwnww", "Full Warm White active.");
      } else if (kelvin < this->cold_white_temperature_ && kelvin > this->neutral_white_temperature_) {
          // Blend between cold white and neutral white
          float blend = (this->cold_white_temperature_ - kelvin) /
                        (this->cold_white_temperature_ - this->neutral_white_temperature_);
          cwhite = brightness * blend;
          nwhite = brightness * (1.0f - blend);
          ESP_LOGI("cwnww", "Blending Cold White (%f) and Neutral White (%f).", cwhite, nwhite);
      } else if (kelvin <= this->neutral_white_temperature_ && kelvin > this->warm_white_temperature_) {
          // Blend between neutral white and warm white
          float blend = (this->neutral_white_temperature_ - kelvin) /
                        (this->neutral_white_temperature_ - this->warm_white_temperature_);
          nwhite = brightness * blend;
          wwhite = brightness * (1.0f - blend);
          ESP_LOGI("cwnww", "Blending Neutral White (%f) and Warm White (%f).", nwhite, wwhite);
      }
  
      // Normalize brightness if constant_brightness_ is set
      if (this->constant_brightness_) {
          float total = cwhite + nwhite + wwhite;
          if (total > 1.0f) {
              cwhite /= total;
              nwhite /= total;
              wwhite /= total;
          }
      }
  
      // Debug log for calculated outputs
      ESP_LOGI("cwnww", "Final Levels -> Cold White: %f, Neutral White: %f, Warm White: %f", cwhite, nwhite, wwhite);
  
      // Apply the levels to the hardware outputs
      this->cold_white_->set_level(cwhite);
      this->neutral_white_->set_level(nwhite);
      this->warm_white_->set_level(wwhite);
  }


 protected:
  output::FloatOutput *cold_white_;
  output::FloatOutput *neutral_white_;
  output::FloatOutput *warm_white_;
  float cold_white_temperature_{3500.0f};
  float neutral_white_temperature_{1800.0f};
  float warm_white_temperature_{1000.0f};
  bool constant_brightness_;
};

}  // namespace cwnww
}  // namespace esphome
