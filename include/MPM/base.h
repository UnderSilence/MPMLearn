#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include <Eigen/StdVector>

#include "settings.h"
namespace mpm {

struct GridAttr {
  T mass_i;
  VT vel_in;
  VT vel_i;
  VT force_i;
  VINT Xi;
};

struct Particle {
  VT pos_p;
  VT vel_p;
  MT F; // deformation gradients
  // MT Fe;
  // MT Fp;
  MT Bp; // for APIC transfer

  // T cp;  // fluids ratio
  // MT Dp; // inertia tensor

  struct MPM_Material *material;
};

struct SimInfo {
  int particle_size = 0;
  int grid_size = 0;
  int grid_w = 0;
  int grid_h = 0;
  int grid_l = 0;

  // simulation factors
  // T E = 50.0f;     // Young's modules
  // T nu = 0.3f;     // Possion ratio
  T alpha = 0.95f; // 0.95 flip/pic

  // T particle_density;
  // T particle_mass;
  // std::string model_path;

  VT gravity = VT::Zero();
  VT world_area = VT::Zero();
  T h = 0.0f;

  T max_velocity = 0.0f;
  unsigned int curr_step = 0;
};

// MT neohookean_piola(T E, T nu, const MT &F);

} // namespace mpm
