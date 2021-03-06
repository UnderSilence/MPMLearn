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
class MPM_Collision;

// neohookean model
class MPM_Simulator {
public:
  MPM_Simulator();
  virtual ~MPM_Simulator();
  enum TransferScheme { FLIP99, FLIP95, APIC };

  void mpm_demo(const std::shared_ptr<MPM_CM> &cm_demo,
                const std::string &output_relative_path = "");

  void mpm_initialize(const VT &gravity, const VT &world_area, T h);

  void add_object(const std::vector<VT> &positions,
                  const std::vector<VT> &velocities, MPM_Material *material);

  void add_object(const std::vector<VT> &positions, MPM_Material *material);

  void set_constitutive_model(const std::shared_ptr<MPM_CM> &cm);
  void set_plasticity(const std::shared_ptr<Plasticity> &plas);

  void set_transfer_scheme(TransferScheme ts);
  // void grid_initialize();
  // void particle_initialize();
  std::vector<VT> get_positions() const;
  T get_max_velocity() const;

  void substep(T dt);
  void clear_simulation();
  void add_collision(const MPM_Collision &coll);

  template <class T, class... Args>
  void add_collision(const T &coll, Args... args) {
    add_collision(coll);
    add_collision(args...);
  }
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
  std::vector<MPM_Collision> colls;
  // std::vector<int> active_nodes;

  void prestep();
  void transfer_P2G();
  void add_gravity();
  // TODO: support variety
  //  E : T, nu : T, F : MT
  void update_grid_force();
  void update_grid_velocity(T dt);
  void update_F(T dt);
  void transfer_G2P();
  void advection(T dt);

  // handle collision
  void solve_particle_collision();
  void solve_grid_collision();
  void solve_grid_boundary(int thickness = 2);
};
} // namespace mpm