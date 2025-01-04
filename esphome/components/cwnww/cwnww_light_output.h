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
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
  void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
    traits.set_min_mireds(this->warm_white_temperature_);
    traits.set_max_mireds(this->cold_white_temperature_);
    return traits;
  }

  void write_state(light::LightState *state) override {
    // Directly map to cold, neutral, and warm white levels
    float cwhite, nwhite, wwhite;
    state->current_values_as_cwww(&cwhite, &nwhite, &wwhite, this->constant_brightness_);

    // Assign values to outputs
    this->cold_white_->set_level(cwhite);
    this->neutral_white_->set_level(nwhite);
    this->warm_white_->set_level(wwhite);

    // Debug logs for final output
    ESP_LOGI("cwnww", "Final Levels -> Cold White: %f, Neutral White: %f, Warm White: %f", cwhite, nwhite, wwhite);
  }

 protected:
  output::FloatOutput *cold_white_;
  output::FloatOutput *neutral_white_;
  output::FloatOutput *warm_white_;
  float cold_white_temperature_{3500.0f};
  float warm_white_temperature_{1000.0f};
  bool constant_brightness_;
};

}  // namespace cwnww
}  // namespace esphome
