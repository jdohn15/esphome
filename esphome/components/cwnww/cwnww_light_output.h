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
  
      // Inform Home Assistant of the proper Kelvin range
      traits.set_min_color_temperature(this->warm_white_temperature_); // Warm white (1000 K)
      traits.set_max_color_temperature(this->cold_white_temperature_); // Cold white (3500 K)
  
      // Debug logs
      ESP_LOGI("cwnww", "Min Kelvin (warm white): %f", this->warm_white_temperature_);
      ESP_LOGI("cwnww", "Max Kelvin (cold white): %f", this->cold_white_temperature_);
  
      return traits;
  }





  void write_state(light::LightState *state) override {
      ESP_LOGI("cwnww", "is_on: %s", state->current_values.is_on() ? "true" : "false");
      ESP_LOGI("cwnww", "Kelvin: %f", state->current_values.get_color_temperature());
      ESP_LOGI("cwnww", "Brightness: %f", state->current_values.get_brightness());
      
      if (!state->current_values.is_on()) {
          // Turn off all channels
          this->cold_white_->set_level(0.0f);
          this->neutral_white_->set_level(0.0f);
          this->warm_white_->set_level(0.0f);
          return;
      }
  
      float cwhite = 0.0f, nwhite = 0.0f, wwhite = 0.0f;
      float kelvin = state->current_values.get_color_temperature();
      float brightness = state->current_values.get_brightness();
  
      // Clamp Kelvin value to the valid range
      if (kelvin < this->warm_white_temperature_) {
          kelvin = this->warm_white_temperature_;
      } else if (kelvin > this->cold_white_temperature_) {
          kelvin = this->cold_white_temperature_;
      }
      ESP_LOGI("cwnww", "Clamped Kelvin: %f", kelvin);
  
      if (kelvin >= this->cold_white_temperature_) {
          // Full cold white
          cwhite = brightness;
      } else if (kelvin <= this->warm_white_temperature_) {
          // Full warm white
          wwhite = brightness;
      } else if (kelvin < this->cold_white_temperature_ && kelvin > this->neutral_white_temperature_) {
          // Blend between cold white and neutral white
          float blend = (this->cold_white_temperature_ - kelvin) / 
                        (this->cold_white_temperature_ - this->neutral_white_temperature_);
          cwhite = brightness * blend;
          nwhite = brightness * (1.0f - blend);
      } else if (kelvin <= this->neutral_white_temperature_ && kelvin > this->warm_white_temperature_) {
          // Blend between neutral white and warm white
          float blend = (this->neutral_white_temperature_ - kelvin) / 
                        (this->neutral_white_temperature_ - this->warm_white_temperature_);
          nwhite = brightness * blend;
          wwhite = brightness * (1.0f - blend);
      }
  
      // Apply constant brightness normalization if configured
      if (this->constant_brightness_) {
          float total = cwhite + nwhite + wwhite;
          if (total > 1.0f) {
              cwhite /= total;
              nwhite /= total;
              wwhite /= total;
          }
      }
  
      // Set the levels for each output
      this->cold_white_->set_level(cwhite);
      this->neutral_white_->set_level(nwhite);
      this->warm_white_->set_level(wwhite);
  
      // Debug logs for verification
      ESP_LOGI("cwnww", "Kelvin: %f, Brightness: %f", kelvin, brightness);
      ESP_LOGI("cwnww", "Cold White: %f, Neutral White: %f, Warm White: %f", cwhite, nwhite, wwhite);
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
