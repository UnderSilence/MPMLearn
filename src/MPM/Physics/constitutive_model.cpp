#include "MPM/Physics/constitutive_model.h"
#include "MPM/material.h"

namespace mpm {
std::tuple<MT, MT> MPM_CM::calc_mixed_stress_tensor(const Particle &particle) {
  return {calc_stress_tensor(particle), MT::Zero()};
}

MT NeoHookean_Piola::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  auto piola = m->mu * (F - F.transpose().inverse()) +
               m->lambda * log(J) * F.transpose().inverse();
  return piola;
}

T NeoHookean_Piola::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  auto log_J = log(J);

  auto psi = 0.5f * m->mu * ((F.transpose() * F).trace() - 3) - m->mu * log_J +
             0.5f * m->lambda * log_J * log_J;
  return psi;
}

MT QuatraticVolumePenalty::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;

  return m->lambda * (J - 1) * J * F.inverse().transpose();
}

T QuatraticVolumePenalty::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;

  return 0.5f * m->lambda * std::pow(J - 1, 2);
}

std::tuple<MT, MT>
QuatraticVolumePenalty::calc_mixed_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;

  return {MT::Zero(), m->lambda * (J - 1) * J * MT::Identity()};
}

MT NeoHookean_Fluid::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  auto piola = m->lambda * log(J) * F.transpose().inverse();
  return piola;
}

T NeoHookean_Fluid::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  auto log_J = log(J);

  auto psi = 0.5f * m->mu * ((F.transpose() * F).trace() - 3) - m->mu * log_J +
             0.5f * m->lambda * log_J * log_J;
  return psi;
}

MT CDMPM_Fluid::calc_stress_tensor(const Particle &particle) {
  throw std::logic_error("function not implement yet");
}

T CDMPM_Fluid::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  auto log_J = log(J);
  // 0.5 * K ( 0.5 * (J^2 - 1) - Log(J))
  auto psi = 0.5f * m->K * (0.5f * (J * J - 1) - log_J);
  return psi;
}

std::tuple<MT, MT>
CDMPM_Fluid::calc_mixed_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = particle.J;
  // d(psi)/d(J) * d(J)/d(F) =
  // 0.5 * K (J - 1 / J) * J * F^{-T}
  // if (J >= 1) {
  //   return MT::Zero();
  // }
  auto piola = 0.5f * m->K * (J - 1 / J) * J * MT::Identity();
  return {MT::Zero(), piola};
}

} // namespace mpm