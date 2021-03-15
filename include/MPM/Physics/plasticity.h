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
  virtual bool projectStrain(Particle &particle) = 0;
};

class vonMises : public Plasticity {
public:
  T yield_stress, fail_stress, xi, alpha;
  vonMises(T yield_stress = 0, T xi = 0, T alpha = 0)
      : yield_stress(yield_stress), xi(xi), alpha(alpha),
        fail_stress(std::sqrt(3.f / 2.f) * yield_stress) {}
  bool projectStrain(Particle &particle);
  std::string name = "vonMises";
};

class Snow : public Plasticity {
public:
  T Jp, psi, theta_c, theta_s, min_Jp, max_Jp;
  Snow(T psi_in = 10, T theta_c_in = 2e-2, T theta_s_in = 7.5e-3,
       T min_Jp_in = 0.6, T max_Jp_in = 20)
      : Jp(1), psi(psi_in), theta_c(theta_c_in), theta_s(theta_s_in),
        min_Jp(min_Jp_in), max_Jp(max_Jp_in) {}

  bool projectStrain(Particle &particle);
  std::string name = "SnowPlasticity";
};
} // namespace mpm