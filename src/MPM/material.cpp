#include "MPM/material.h"
#include "MPM/Math/matrix_solver.h"

namespace mpm {

MPM_Material::MPM_Material(float E, float nu, float mass, float density)
    : E(E), nu(nu), mass(mass), density(density) {
  mu = 0.5f * E / (1 + nu);
  lambda = E * nu / (1 + nu) / (1 - 2 * nu);
  volume = mass / density;
  K = E / (1 - 2 * nu) / 3.0f;
}

} // namespace mpm
