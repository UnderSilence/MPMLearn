#include "MPM/Physics/plasticity.h"
#include "MPM/Math/matrix_solver.h"
#include "MPM/material.h"

namespace mpm {
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