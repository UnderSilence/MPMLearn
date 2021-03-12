#include "MPM/mpm_mtl.h"
#include "Math/matrix_solver.h"

namespace mpm {

MPM_Material::MPM_Material(float E, float nu, float mass, float density)
    : E(E), nu(nu), mass(mass), density(density) {
  mu = 0.5f * E / (1 + nu);
  lambda = E * nu / (1 + nu) / (1 - 2 * nu);
  volume = mass / density;
  K = E / (1 - 2 * nu) / 3.0f;
}

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
  auto piola = 0.5f * m->K * (J - 1 / J) * J * Matrix3f::Identity();
  return {Matrix3f::Zero(), piola};
}

// yield surface

bool vonMises::projectStrain(Particle &particle) {
  auto c = particle.material;
  auto &F = particle.F;
  Matrix3f U, Sigma, V;
  SVDSolver(F, U, Sigma, V);

  Vector3f epsilon = hatOfMatrix(Sigma).array().max(1e-4).log();
  float trace_epsilon = epsilon.sum();
  Vector3f epsilon_dev = epsilon - (trace_epsilon / 3) * Vector3f::Ones();
  float epsilon_dev_norm = epsilon_dev.norm();

  float delta_gamma = epsilon_dev_norm - yield_stress / (2 * c->mu);
  if (delta_gamma < 0) { // case I
    return false;
  }
  alpha += sqrt(2.f / 3.f) * delta_gamma;

  // hardening
  yield_stress += xi * delta_gamma;

  Vector3f H =
      epsilon - delta_gamma / epsilon_dev_norm * epsilon_dev; // case II
  F = U * vectorToMatrix(H.array().exp()) * V.transpose();
  return true;
}

bool Snow::projectStrain(Particle &particle) {
  auto c = particle.material;
  auto &F = particle.F;
  Matrix3f U, V;
  Vector3f Sigma;
  SVDSolverDiagonal(F, U, Sigma, V);

  float Fe_det = 1.f;
  for (int i = 0; i < 3; i++) {
    Sigma(i) = std::max(std::min(Sigma(i), 1.f + theta_s), 1.f - theta_c);
    Fe_det *= Sigma(i);
  }

  Matrix3f sigma_m = vectorToMatrix(Sigma);
  Matrix3f Fe = U * sigma_m * V.transpose();
  // T Jp_new = std::max(std::min(Jp * F.determinant() / Fe_det, max_Jp),
  // min_Jp);
  float Jp_new = Jp * F.determinant() / Fe_det;
  if (!(Jp_new <= max_Jp))
    Jp_new = max_Jp;
  if (!(Jp_new >= min_Jp))
    Jp_new = min_Jp;

  F = Fe;
  /*c->mu *= std::exp(psi * (Jp - Jp_new));
  c->lambda *= std::exp(psi * (Jp - Jp_new));*/
  Jp = Jp_new;

  return false;
}

} // namespace mpm
