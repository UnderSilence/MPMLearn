#include "MPM/mpm_mtl.h"

namespace mpm {
MPM_Material::MPM_Material(float E, float nu, float mass, float density)
    : E(E), nu(nu), mass(mass), density(density) {
  mu = 0.5f * E / (1 + nu);
  lambda = E * nu / (1 + nu) / (1 - 2 * nu);
  volume = mass / density;
}

Matrix3f NeoHookean_Piola::calc_stress_tensor(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto piola = m->mu * (F - F.transpose().inverse()) +
               m->lambda * log(J) * F.transpose().inverse();
  return piola;
}

float NeoHookean_Piola::calc_phi(const Particle &particle) {
  auto m = particle.material;
  auto F = particle.F;
  auto J = F.determinant();
  auto log_J = log(J);

  auto phi = 0.5f * m->mu * ((F.transpose() * F).trace() - 3) - m->mu * log_J +
             0.5f * m->lambda * log_J * log_J;
  return phi;
}

} // namespace mpm