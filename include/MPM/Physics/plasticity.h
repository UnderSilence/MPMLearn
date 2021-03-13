#pragma once

#include "MPM/base.h"
#include <string>

namespace mpm {

// Yield Surface
class Plasticity {
public:
  Plasticity() {}
  virtual ~Plasticity() {}
  std::string name;
  virtual bool projectStrain(Particle &particle) { return false; }
};

class vonMises : public Plasticity {
public:
  float yield_stress, fail_stress, xi, alpha;
  vonMises(float yield_stress = 0, float xi = 0, float alpha = 0)
      : yield_stress(yield_stress), xi(xi), alpha(alpha),
        fail_stress(std::sqrt(3.f / 2.f) * yield_stress) {}
  bool projectStrain(Particle &particle);
  std::string name = "vonMises";
};

class Snow : public Plasticity {
public:
  float Jp, psi, theta_c, theta_s, min_Jp, max_Jp;
  Snow(float psi_in = 10, float theta_c_in = 2e-2, float theta_s_in = 7.5e-3,
       float min_Jp_in = 0.6, float max_Jp_in = 20)
      : Jp(1), psi(psi_in), theta_c(theta_c_in), theta_s(theta_s_in),
        min_Jp(min_Jp_in), max_Jp(max_Jp_in) {}

  bool projectStrain(Particle &particle);
  std::string name = "SnowPlasticity";
};
} // namespace mpm