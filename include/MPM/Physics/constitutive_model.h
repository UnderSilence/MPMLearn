#pragma once

#include "MPM/base.h"

namespace mpm {
// Constitutive Model for simulation
class MPM_CM {
public:
  virtual Matrix3f calc_stress_tensor(const Particle &particle) = 0;
  virtual float calc_psi(const Particle &particle) = 0;
  virtual std::tuple<Matrix3f, Matrix3f>
  calc_mixed_stress_tensor(const Particle &particle);
};

class NeoHookean_Piola : public MPM_CM {
  virtual Matrix3f calc_stress_tensor(const Particle &particle);
  virtual float calc_psi(const Particle &particle);
};

class QuatraticVolumePenalty : public MPM_CM {
  virtual Matrix3f calc_stress_tensor(const Particle &particle);
  virtual float calc_psi(const Particle &particle);
  virtual std::tuple<Matrix3f, Matrix3f>
  calc_mixed_stress_tensor(const Particle &particle);
};

class NeoHookean_Fluid : public MPM_CM {
  virtual Matrix3f calc_stress_tensor(const Particle &particle);
  virtual float calc_psi(const Particle &particle);
};

class CDMPM_Fluid : public MPM_CM {
  virtual Matrix3f calc_stress_tensor(const Particle &particle);
  virtual float calc_psi(const Particle &particle);
  virtual std::tuple<Matrix3f, Matrix3f>
  calc_mixed_stress_tensor(const Particle &particle);
};
} // namespace mpm