#pragma once

#include "MPM/base.h"
#include "MPM/material.h"
#include "tbb/concurrent_vector.h"
#include "tbb/spin_mutex.h"

namespace mpm {

struct Particle;
struct GridAttr;
struct SimInfo;
struct MPM_Material;
class MPM_CM;
class Plasticity;

// neohookean model
class MPM_Simulator {
public:
  MPM_Simulator();
  virtual ~MPM_Simulator();
  enum TransferScheme { FLIP99, FLIP95, APIC };

  void mpm_demo(const std::shared_ptr<MPM_CM> &cm_demo,
                const std::string &output_relative_path = "");

  void mpm_initialize(const Vector3f &gravity, const Vector3f &world_area,
                      float h);

  void add_object(const std::vector<Vector3f> &positions,
                  const std::vector<Vector3f> &velocities,
                  MPM_Material *material);

  void add_object(const std::vector<Vector3f> &positions,
                  MPM_Material *material);

  void set_constitutive_model(const std::shared_ptr<MPM_CM> &cm);
  void set_plasticity(const std::shared_ptr<Plasticity> &plas);

  void set_transfer_scheme(TransferScheme ts);
  // void grid_initialize();
  // void particle_initialize();
  std::vector<Vector3f> get_positions() const;
  float get_max_velocity() const;

  void substep(float dt);
  void clear_simulation();
  // bool export_result(const std::string &export_path, int curr_frame);

private:
  SimInfo sim_info;
  Particle *particles;
  GridAttr *grid_attrs;

  tbb::spin_mutex *grid_mutexs;

  // MPM simulation consititutive model
  TransferScheme transfer_scheme = TransferScheme::APIC;
  std::shared_ptr<MPM_CM> cm;
  std::shared_ptr<Plasticity> plasticity;

  // storage the degree of freedoms
  tbb::concurrent_vector<int> active_nodes;
  // std::vector<int> active_nodes;

  void prestep();
  void transfer_P2G();
  void add_gravity();
  // TODO: support variety
  //  E : float, nu : float, F : Matrix3f
  void update_grid_force();
  void update_grid_velocity(float dt);
  void update_F(float dt);
  void transfer_G2P();
  void advection(float dt);

  // handle collision
  void solve_particle_collision();
  void solve_grid_collision();
  void solve_grid_boundary(int thickness = 2);
};
} // namespace mpm