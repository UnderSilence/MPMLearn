#pragma once

namespace mpm {
struct MPM_Material {
  // lame params
  T E;  // young's modulus
  T nu; // possion ratio
  T mass;
  T density;

  T lambda;
  T mu;
  T K;      // bulk modulus
  T volume; // particle's volume at time 0
  MPM_Material(T E, T nu, T mass, T density);
};

} // namespace mpm
