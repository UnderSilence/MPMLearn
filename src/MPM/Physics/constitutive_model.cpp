#include "MPM/Physics/constitutive_model.h"
#include "MPM/material.h"

namespace mpm {
std::tuple<Matrix3f, Matrix3f>
MPM_CM::calc_mixed_stress_tensor(const Particle &particle) {
  return {calc_stress_tensor(particle), Matrix3f::Zero()};
}

Matrix3f NeoHookean_Piola::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto piola = m->mu * (F - F.transpose().inverse()) +
               m->lambda * log(J) * F.transpose().inverse();
  return piola;
}

float NeoHookean_Piola::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto log_J = log(J);

  auto psi = 0.5f * m->mu * ((F.transpose() * F).trace() - 3) - m->mu * log_J +
             0.5f * m->lambda * log_J * log_J;
  return psi;
}

Matrix3f QuatraticVolumePenalty::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();

  return 0.5f * m->lambda * 2 * (J - 1) * J * F.inverse().transpose();
}

float QuatraticVolumePenalty::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();

  return 0.5f * m->lambda * std::pow(J - 1, 2);
}

std::tuple<Matrix3f, Matrix3f>
QuatraticVolumePenalty::calc_mixed_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = std::max(1e-4f, F.determinant());
  return {Matrix3f::Zero(), m->lambda * (J - 1) * J * Matrix3f::Identity()};
}

Matrix3f NeoHookean_Fluid::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto piola = m->lambda * log(J) * F.transpose().inverse();
  return piola;
}

float NeoHookean_Fluid::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto log_J = log(J);

  auto psi = 0.5f * m->mu * ((F.transpose() * F).trace() - 3) - m->mu * log_J +
             0.5f * m->lambda * log_J * log_J;
  return psi;
}

Matrix3f CDMPM_Fluid::calc_stress_tensor(const Particle &particle) {
  throw std::logic_error("function not implement yet");
}

float CDMPM_Fluid::calc_psi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto log_J = log(J);
  // 0.5 * K ( 0.5 * (J^2 - 1) - Log(J))
  auto psi = 0.5f * m->K * (0.5f * (J * J - 1) - log_J);
  return psi;
}

std::tuple<Matrix3f, Matrix3f>
CDMPM_Fluid::calc_mixed_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  // d(psi)/d(J) * d(J)/d(F) =
  // 0.5 * K (J - 1 / J) * J * F^{-T}
  // if (J >= 1) {
  //   return Matrix3f::Zero();
  // }
  auto piola =
      0.5f * m->K * (J - 1 / std::max(1e-5f, J)) * J * Matrix3f::Identity();
  return {Matrix3f::Zero(), piola};
}

} // namespace mpm