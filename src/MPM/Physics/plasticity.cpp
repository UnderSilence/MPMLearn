#include "MPM/Physics/plasticity.h"
#include "MPM/Math/matrix_solver.h"
#include "MPM/material.h"

namespace mpm {
// yield surface

bool vonMises::projectStrain(Particle &particle) {
  auto c = particle.material;
  auto &F = particle.F;
  MT U, Sigma, V;
  SVDSolver(F, U, Sigma, V);

  VT epsilon = hatOfMatrix(Sigma).array().max(1e-4).log();
  T trace_epsilon = epsilon.sum();
  VT epsilon_dev = epsilon - (trace_epsilon / 3) * VT::Ones();
  T epsilon_dev_norm = epsilon_dev.norm();

  T delta_gamma = epsilon_dev_norm - yield_stress / (2 * c->mu);
  if (delta_gamma < 0) { // case I
    return false;
  }
  alpha += sqrt(2.f / 3.f) * delta_gamma;

  // hardening
  yield_stress += xi * delta_gamma;

  VT H = epsilon - delta_gamma / epsilon_dev_norm * epsilon_dev; // case II
  F = U * vectorToMatrix(H.array().exp()) * V.transpose();
  return true;
}

bool Snow::projectStrain(Particle &particle) {
  auto c = particle.material;
  auto &F = particle.F;
  MT U, V;
  VT Sigma;
  SVDSolverDiagonal(F, U, Sigma, V);

  T Fe_det = 1.f;
  for (int i = 0; i < 3; i++) {
    Sigma(i) = std::max(std::min(Sigma(i), T(1) + theta_s), T(1) - theta_c);
    Fe_det *= Sigma(i);
  }

  MT sigma_m = vectorToMatrix(Sigma);
  MT Fe = U * sigma_m * V.transpose();
  // T Jp_new = std::max(std::min(Jp * F.determinant() / Fe_det, max_Jp),
  // min_Jp);
  T Jp_new = Jp * F.determinant() / Fe_det;
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