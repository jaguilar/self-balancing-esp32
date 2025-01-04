#include "intpid.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <format>

namespace intpid {
namespace {

class WaterHeaterModel {
 public:
  WaterHeaterModel(float water_mass_g, float r_factor, float input_temp_celcius,
                   float max_power_joules)
      : water_mass_g_(water_mass_g),
        r_factor_(r_factor),
        input_temp_(input_temp_celcius),
        max_power_(max_power_joules),
        // Default setpoint is 130f in celcius.
        setpoint_(54),
        water_temp_(input_temp_celcius),
        power_fraction_(0),
        flow_rate_(0) {}

  float temp() const { return water_temp_; }

  float setpoint() const { return setpoint_; }
  void set_setpoint(float setpoint) { setpoint_ = setpoint; }

  // 0 to 1
  void set_power(float power_fraction) {
    assert(power_fraction >= 0 && power_fraction <= 1);
    power_fraction_ = power_fraction;
  }

  // Sets the number of grams of water per second draining
  // from the tank.
  void set_flow_rate(float flow_rate) { flow_rate_ = flow_rate; }

  // Updates the state of the water heater as if the given number of
  // seconds have passed.
  void Update(int32_t dt_secs) {
    // Calculate the temperature lost to water draining.
    const float grams_drained = flow_rate_ * dt_secs;
    const float delta_c_drain =
        (grams_drained / water_mass_g_) * (input_temp_ - water_temp_);

    // Calculate the temperature lost to the environment.
    constexpr float surface_area_m2 = 2.5f;
    constexpr float air_temp = 20.0f;
    const float heat_lost_to_air =
        surface_area_m2 * r_factor_ * (water_temp_ - air_temp) * dt_secs;
    constexpr float water_specific_heat = 4.186f;  // J/g/C
    const float delta_c_air =
        -heat_lost_to_air / water_mass_g_ / water_specific_heat;

    // Calculate the heat gained from the heating element.
    const float energy_from_heater = power_fraction_ * max_power_ * dt_secs;
    const float delta_c_heater =
        energy_from_heater / water_mass_g_ / water_specific_heat;

    water_temp_ = water_temp_ + delta_c_drain + delta_c_air + delta_c_heater;
  }

 private:
  const float water_mass_g_;
  const float r_factor_;
  const float input_temp_;
  const float max_power_;

  int32_t setpoint_;
  float water_temp_;
  float power_fraction_;
  float flow_rate_;
};

TEST(IntPid, BasicTest) {
#ifdef ARDUINO
  GTEST_SKIP();
#endif
  constexpr int dt = 60;

  FILE* f = fopen("/tmp/test.csv", "w");
  ASSERT_TRUE(f);

  WaterHeaterModel model(200'000, 20, 20, 11700);
  auto pid = intpid::Pid::Create(intpid::Config{.gain = 15,
                                                .integral_time = 85,
                                                .derivative_time = 7,
                                                .setpoint_min = 0,
                                                .setpoint_max = 100,
                                                .measurement_min = 0,
                                                .measurement_max = 100,
                                                .output_min = 0,
                                                .output_max = 100,
                                                .timestep_max = 60});
  ASSERT_TRUE(pid) << pid.error();

  for (int t = 0; t < 12 * 60 * 60; t += dt) {
    if (t > 4 * 60 * 60 && t < 5 * 60 * 60) {
      // One shower at about 75% hot water (160g/s water flow total).
      model.set_flow_rate(80);
    } else if (t > 5 * 60 * 60 && t < 8 * 60 * 60) {
      model.set_flow_rate(10);
    } else if (t > 8 * 60 * 60 && t < 9 * 60 * 60) {
      // Every shower in the house is on! Whoa!
      model.set_flow_rate(4 * 80);
    }

    pid->set_setpoint(model.setpoint());
    const auto power = pid->Update(model.temp(), dt);
    model.set_power(float{power} / 100.0);
    model.Update(dt);

    fprintf(f, "%s",
            std::format("{},{},{},{},{},{},{}\n", t, model.setpoint(),
                        model.temp(), float{pid->sum()}, float{pid->p()},
                        float{pid->i()}, float{pid->d()})
                .c_str());
  }
  fclose(f);
}

}  // namespace
}  // namespace intpid
