#pragma once

#include "MPM/base.h"

namespace mpm {
// Constitutive Model for simulation
class MPM_CM {
public:
  virtual MT calc_stress_tensor(const Particle &particle) = 0;
  virtual T calc_psi(const Particle &particle) = 0;
  virtual std::tuple<MT, MT> calc_mixed_stress_tensor(const Particle &particle);
};

class NeoHookean_Piola : public MPM_CM {
  virtual MT calc_stress_tensor(const Particle &particle);
  virtual T calc_psi(const Particle &particle);
};

class QuatraticVolumePenalty : public MPM_CM {
  virtual MT calc_stress_tensor(const Particle &particle);
  virtual T calc_psi(const Particle &particle);
  virtual std::tuple<MT, MT> calc_mixed_stress_tensor(const Particle &particle);
};

class NeoHookean_Fluid : public MPM_CM {
  virtual MT calc_stress_tensor(const Particle &particle);
  virtual T calc_psi(const Particle &particle);
};

class CDMPM_Fluid : public MPM_CM {
  virtual MT calc_stress_tensor(const Particle &particle);
  virtual T calc_psi(const Particle &particle);
  virtual std::tuple<MT, MT> calc_mixed_stress_tensor(const Particle &particle);
};
} // namespace mpm