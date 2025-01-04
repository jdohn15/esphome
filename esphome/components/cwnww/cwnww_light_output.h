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
    traits.set_min_color_temperature(this->warm_white_temperature_);
    traits.set_max_color_temperature(this->cold_white_temperature_);
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

    float kelvin = state->current_values.get_color_temperature();
    float brightness = state->current_values.get_brightness();

    float cwhite = 0.0f, nwhite = 0.0f, wwhite = 0.0f;

    // Full cold white
    if (kelvin >= this->cold_white_temperature_) {
      cwhite = brightness;
    }
    // Full warm white
    else if (kelvin <= this->warm_white_temperature_) {
      wwhite = brightness;
    }
    // Blend cold white and neutral white
    else if (kelvin > this->neutral_white_temperature_) {
      float blend = (this->cold_white_temperature_ - kelvin) /
                    (this->cold_white_temperature_ - this->neutral_white_temperature_);
      cwhite = brightness * blend;
      nwhite = brightness * (1.0f - blend);
    }
    // Blend neutral white and warm white
    else {
      float blend = (this->neutral_white_temperature_ - kelvin) /
                    (this->neutral_white_temperature_ - this->warm_white_temperature_);
      nwhite = brightness * blend;
      wwhite = brightness * (1.0f - blend);
    }

    // Apply constant brightness normalization
    if (this->constant_brightness_) {
      float total = cwhite + nwhite + wwhite;
      if (total > 1.0f) {
        cwhite /= total;
        nwhite /= total;
        wwhite /= total;
      }
    }

    // Set levels
    this->cold_white_->set_level(cwhite);
    this->neutral_white_->set_level(nwhite);
    this->warm_white_->set_level(wwhite);

    // Debug logs
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
