#pragma once

namespace mpm {
struct MPM_Material {
  // lame params
  float E;  // young's modulus
  float nu; // possion ratio
  float mass;
  float density;

  float lambda;
  float mu;
  float K;      // bulk modulus
  float volume; // particle's volume at time 0
  MPM_Material(float E, float nu, float mass, float density);
};

} // namespace mpm
