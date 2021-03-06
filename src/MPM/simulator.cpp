#include "MPM/simulator.h"
#include "MPM/Math/interpolation.h"
#include "MPM/Physics/constitutive_model.h"
#include "MPM/Physics/plasticity.h"
#include "MPM/collision.h"

#include "MPM/Utils/io.h"
#include "MPM/mpm_pch.h"

#include <tbb/parallel_for.h>
#include <tbb/parallel_reduce.h>
#include <tbb/spin_mutex.h>

namespace mpm {

MPM_Simulator::MPM_Simulator()
    : sim_info(), particles(nullptr), grid_attrs(nullptr),
      grid_mutexs(nullptr) {}

MPM_Simulator::~MPM_Simulator() {
  // note: build a active_id <-> grid_id map in future
  //      for lower memory usage
  if (particles) {
    delete[] particles;
    particles = nullptr;
  }
  if (grid_attrs) {
    delete[] grid_attrs;
    grid_attrs = nullptr;
  }
  if (grid_mutexs) {
    delete[] grid_mutexs;
    grid_mutexs = nullptr;
  }
}

void MPM_Simulator::clear_simulation() {
  if (particles) {
    delete[] particles;
    particles = nullptr;
  }
  if (grid_attrs) {
    delete[] grid_attrs;
    grid_attrs = nullptr;
  }
  if (grid_mutexs) {
    delete[] grid_mutexs;
    grid_mutexs = nullptr;
  }
  sim_info = SimInfo();
}

/*
void MPM_Simulator::mpm_demo(const std::shared_ptr<MPM_CM> &cm_demo,
                             const std::string &output_relative_path) {

  clear_simulation();
  VT gravity{0.0f, -9.8f, 0.0f};
  VT area{1.0f, 1.0f, 1.0f};
  VT velocity{-0.5f, 0.5f, -0.3f};

  T h = 0.02f;
  auto mtl_jello = new MPM_Material(50.0f, 0.3f, 10.0f, 1.0f);
  auto mtl_water = new MPM_Material(50.0f, 0.3f, 8e-6f, 1.0f);

  mpm_initialize(gravity, area, h);
  cm = cm_demo;

  std::vector<VT> positions;
  auto model_path = "../models/dense_cube.obj";

  if (read_particles(model_path, positions)) {
    MPM_INFO("read in particles from {} SUCCESS", model_path);
    add_object(positions, mtl_water);
  }

  int frame_rate = 60;
  T dt = 1e-3f;
  int total_frame = 300;
  int steps_per_frame = (int)ceil((1.0f / frame_rate) / dt);

  MPM_INFO("Simulation start, Meta Informations:\n"
           "\tframe_rate: {}\n"
           "\tdt: {}\n"
           "\tsteps_per_frame: {}\n",
           frame_rate, dt, steps_per_frame);

  // export frame#0 first
  auto output_dir = "../output/" + output_relative_path;
  write_particles(output_dir + "0.bgeo", get_positions());
  for (int frame = 0; frame < total_frame;) {
    {
      MPM_SCOPED_PROFILE("frame#" + std::to_string(frame + 1));
      for (int i = 0; i < steps_per_frame; i++) {
        // MPM_INFO("begin step {}", i);
        substep(dt);
      }
    }
    write_particles(output_dir + std::to_string(++frame) + ".bgeo",
                    get_positions());
    // export_result("../output/", ++frame);
  }
}*/

void MPM_Simulator::substep(T dt) {
  // MPM_SCOPED_PROFILE_FUNCTION();
  // TODO: add profiler later
  MPM_ASSERT(cm, "PLEASE SET CONSTITUTIVE_MODEL BEFORE SIMULATION");

  prestep();
  transfer_P2G();
  add_gravity();
  update_grid_force();
  update_grid_velocity(dt);
  solve_grid_boundary(2);
  solve_grid_collision();
  update_F(dt);
  transfer_G2P();
  advection(dt);
  solve_particle_collision();
  sim_info.curr_step++;
}

std::vector<VT> MPM_Simulator::get_positions() const {
  std::vector<VT> positions(sim_info.particle_size);
  tbb::parallel_for(0, sim_info.particle_size,
                    [&](int i) { positions[i] = particles[i].pos_p; });
  return positions;
}

T MPM_Simulator::get_max_velocity() const { return sim_info.max_velocity; }

// bool MPM_Simulator::export_result(const std::string &export_dir,
//                                   int curr_frame) {
//   // MPM_INFO("export frame_{}'s result.", curr_frame);
//   std::vector<VT> positions(sim_info.particle_size);

//   tbb::parallel_for(0, sim_info.particle_size,
//                     [&](int i) { positions[i] = particles[i].pos_p; });

//   auto export_path = export_dir + std::to_string(curr_frame) + ".bgeo";

//   return write_particles(export_path, positions);
// }

void MPM_Simulator::mpm_initialize(const VT &gravity, const VT &world_area,
                                   T h) {
  sim_info.h = h;
  sim_info.gravity = gravity;
  sim_info.world_area = world_area;

  // grid_initialize
  auto W = static_cast<int>(world_area[0] / h + 1);
  auto H = static_cast<int>(world_area[1] / h + 1);
  auto L = static_cast<int>(world_area[2] / h + 1);

  sim_info.grid_w = W;
  sim_info.grid_h = H;
  sim_info.grid_l = L;
  sim_info.grid_size = W * H * L;
  grid_attrs = new GridAttr[sim_info.grid_size];
  grid_mutexs = new tbb::spin_mutex[sim_info.grid_size];

  MPM_INFO("MPM simulation space info:\n"
           "\tgrid_size: {}->{}x{}x{}\n"
           "\tgrid dx: {}\n"
           "\tgrid gravity: {}\n"
           "\tworld area: {}",
           sim_info.grid_size, W, H, L, sim_info.h,
           sim_info.gravity.transpose(), sim_info.world_area.transpose());

  for (int i = 0; i < W; i++)
    for (int j = 0; j < H; j++)
      for (int k = 0; k < L; k++) {
        int index = i * H * L + j * L + k;
        grid_attrs[index].mass_i = 0;
        grid_attrs[index].force_i = VT::Zero();
        grid_attrs[index].vel_i = VT::Zero();
        grid_attrs[index].vel_in = VT::Zero();
        grid_attrs[index].Xi = VINT(i, j, k);
      }
}

void MPM_Simulator::add_collision(const MPM_Collision &coll) {
  colls.emplace_back(coll);
}

void MPM_Simulator::add_object(const std::vector<VT> &positions,
                               const std::vector<VT> &velocities,
                               MPM_Material *material) {

  MPM_ASSERT(positions.size() == velocities.size() && material != nullptr,
             "PLEASE CHECK OBJECT's POSITION SIZE IFF EQUALS VELOCITY SIZE");
  auto new_size = sim_info.particle_size + positions.size();

  if (particles) {
    Particle *new_particles = new Particle[new_size];
    memcpy(new_particles, particles, sim_info.particle_size * sizeof(Particle));
    delete[] particles;
    particles = new_particles;
  } else {
    particles = new Particle[new_size];
  }

  for (auto i = sim_info.particle_size; i < new_size; i++) {
    particles[i].pos_p = positions[i - sim_info.particle_size];
    particles[i].vel_p = velocities[i - sim_info.particle_size];
    particles[i].F = MT::Identity();
    particles[i].J = 1;
    // particles[i].Fe = MT::Identity();
    // particles[i].Fp = MT::Identity();
    particles[i].Bp = MT::Zero();
    particles[i].material = material;
  }

  sim_info.particle_size = new_size;
}

void MPM_Simulator::add_object(const std::vector<VT> &positions,
                               MPM_Material *material) {
  MPM_ASSERT(material != nullptr, "MATERIAL SHOULD NOT BE NULLPTR");
  auto new_size = sim_info.particle_size + positions.size();

  if (particles) {
    Particle *new_particles = new Particle[new_size];
    memcpy(new_particles, particles, sim_info.particle_size * sizeof(Particle));
    delete[] particles;
    particles = new_particles;
  } else {
    particles = new Particle[new_size];
  }

  for (auto i = sim_info.particle_size; i < new_size; i++) {
    particles[i].pos_p = positions[i - sim_info.particle_size];
    particles[i].F = MT::Identity();
    // particles[i].Fe = MT::Identity();
    // particles[i].Fp = MT::Identity();
    particles[i].Bp = MT::Zero();
    particles[i].material = material;
  }
  sim_info.particle_size = new_size;
}

void MPM_Simulator::set_constitutive_model(const std::shared_ptr<MPM_CM> &cm) {
  this->cm = cm;
}

void MPM_Simulator::set_plasticity(const std::shared_ptr<Plasticity> &plas) {
  this->plasticity = plas;
}

void MPM_Simulator::set_transfer_scheme(TransferScheme ts) {
  this->transfer_scheme = ts;
  if (ts == TransferScheme::FLIP95) {
    sim_info.alpha = 0.95f;
  } else if (ts == TransferScheme::FLIP99) {
    sim_info.alpha = 0.99f;
  }
}

void MPM_Simulator::prestep() {
  // MPM_SCOPED_PROFILE_FUNCTION();
  /*
  @MetaRu: follow parts calculate M : inertia tensor
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int iter) {
    // for (int iter = 0; iter < sim_info.particle_size; iter++) {
    // convert particles position to grid space by divide h
    // particle position in grid space
    VT gs_particle_pos = particles[iter].pos_p / sim_info.h;
    auto [base_node, wp, dwp] = quatratic_interpolation(gs_particle_pos);

    auto &particle = particles[iter];
    auto &mass_p = particle.material->mass;
    particle.Dp = MT::Zero();

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
          // note: do not use auto here (cause error in release mode)
          VINT curr_node = base_node + VINT(i, j, k);
          int index = curr_node(0) * sim_info.grid_h * sim_info.grid_l +
                      curr_node(1) * sim_info.grid_l + curr_node(2);
          T wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);
          VT dxip = curr_node.cast<T>() - gs_particle_pos;
          particle.Dp += wijk * dxip * dxip.transpose();
        }

    MPM_INFO("particle {}'s postision = {}, Dp = \n{}", iter,
            particle.pos_p.transpose(), particle.Dp);
  });
  */

  tbb::parallel_for(0, sim_info.grid_size, [&](int i) {
    grid_attrs[i].mass_i = 0;
    grid_attrs[i].force_i = VT::Zero();
    grid_attrs[i].vel_i = VT::Zero();
    grid_attrs[i].vel_in = VT::Zero();
  });
  active_nodes.resize(0);
  sim_info.max_velocity = 0.0f;
}

void MPM_Simulator::transfer_P2G() {
  // MPM_SCOPED_PROFILE_FUNCTION();
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int iter) {
    // for (int iter = 0; iter < sim_info.particle_size; iter++) {
    // convert particles position to grid space by divide h
    // particle position in grid space
    VT particle_pos = particles[iter].pos_p;
    auto inv_h = 1.0f / sim_info.h;
    auto [base_node, wp, dwp] = quatratic_interpolation(particle_pos * inv_h);

    auto particle = particles[iter];
    auto mass_p = particle.material->mass;

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
          // note: do not use auto here (cause error in release mode)
          VINT curr_node = base_node + VINT(i, j, k);
          int index = curr_node(0) * sim_info.grid_h * sim_info.grid_l +
                      curr_node(1) * sim_info.grid_l + curr_node(2);

          // check if particles run out of boundaries
          MPM_ASSERT(0 <= index && index < sim_info.grid_size,
                     " PARTICLE[{}] OUT OF GRID at Transfer_P2G\n"
                     "\tposition: {}"
                     "\tvelocity: {}",
                     iter, particle.pos_p.transpose(),
                     particle.vel_p.transpose());

          T wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);
          VT plus = VT::Zero();
          if (transfer_scheme == TransferScheme::APIC) {
            plus = particles[iter].Bp * 4 *
                   (curr_node.cast<T>() - particle_pos * inv_h);
          }

          {
            // critical section
            tbb::spin_mutex::scoped_lock lock(grid_mutexs[index]);

            // accumulate momentum at time n
            grid_attrs[index].vel_in +=
                wijk * mass_p * (particles[iter].vel_p + plus);
            grid_attrs[index].mass_i += wijk * mass_p;
          }
        }
  });

  tbb::parallel_for(0, (int)sim_info.grid_size, [&](int iter) {
    if (grid_attrs[iter].mass_i != T(0)) {
      {
        // critical section
        active_nodes.push_back(iter);
      }
      grid_attrs[iter].vel_in =
          grid_attrs[iter].vel_in / grid_attrs[iter].mass_i;
    } else {
      grid_attrs[iter].vel_in = VT::Zero();
    }
  });
} // namespace mpm

void MPM_Simulator::add_gravity() {
  // MPM_SCOPED_PROFILE_FUNCTION();
  // MPM_ASSERT(active_nodes.size() < sim_info.grid_size);
  tbb::parallel_for(0, (int)active_nodes.size(), [&](int i) {
    int index = active_nodes[i];
    grid_attrs[index].force_i += sim_info.gravity * grid_attrs[index].mass_i;
  });
}

void MPM_Simulator::update_grid_force() {
  // update grid forcing from particles F(deformation gradients)

  // MPM_SCOPED_PROFILE_FUNCTION();
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int iter) {
    // for (int iter = 0; iter < sim_info.particle_size; iter++) {
    auto F = particles[iter].F;
    auto vol_p = particles[iter].material->volume;
    auto inv_h = 1.0f / sim_info.h;

    // return [F_based_stress and J_based_stress]
    // to reduce numerical calculation errors
    // all in first piola-kirchhoff stress
    auto [stress_F, stress_J] = cm->calc_mixed_stress_tensor(particles[iter]);

    auto [base_node, wp, dwp] =
        quatratic_interpolation(particles[iter].pos_p * inv_h);
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
          VINT curr_node = base_node + VINT(i, j, k);
          VT grad_wip{dwp(i, 0) * wp(j, 1) * wp(k, 2) * inv_h,
                      wp(i, 0) * dwp(j, 1) * wp(k, 2) * inv_h,
                      wp(i, 0) * wp(j, 1) * dwp(k, 2) * inv_h};

          auto index = curr_node.x() * sim_info.grid_h * sim_info.grid_l +
                       curr_node.y() * sim_info.grid_l + curr_node.z();
          MPM_ASSERT(0 <= index && index < sim_info.grid_size,
                     "PARTICLE OUT OF GRID");

          {
            // critical section
            tbb::spin_mutex::scoped_lock lock(grid_mutexs[index]);

            grid_attrs[index].force_i -=
                vol_p * (stress_F * F.transpose() + stress_J) * grad_wip;
          }
        }
  });
}

void MPM_Simulator::update_grid_velocity(T dt) {
  // MPM_SCOPED_PROFILE_FUNCTION();
  tbb::parallel_for(0, (int)active_nodes.size(), [&](int i) {
    int index = active_nodes[i];
    // vel_n+1 = vel_n + f_i / m_i * dt
    grid_attrs[index].vel_i =
        grid_attrs[index].vel_in +
        dt * grid_attrs[index].force_i / grid_attrs[index].mass_i;
  });
}

void MPM_Simulator::update_F(T dt) {
  // MPM_SCOPED_PROFILE_FUNCTION();
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int iter) {
    // for (int iter = 0; iter < sim_info.particle_size; iter++) {
    auto F = particles[iter].F;
    auto J = particles[iter].J;
    auto inv_h = 1.0f / sim_info.h;
    auto [base_node, wp, dwp] =
        quatratic_interpolation(particles[iter].pos_p * inv_h);

    MT grad_v = MT::Zero();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
          VINT curr_node = base_node + VINT(i, j, k);
          VT grad_wip{dwp(i, 0) * wp(j, 1) * wp(k, 2) * inv_h,
                      wp(i, 0) * dwp(j, 1) * wp(k, 2) * inv_h,
                      wp(i, 0) * wp(j, 1) * dwp(k, 2) * inv_h};

          auto index = curr_node(0) * sim_info.grid_h * sim_info.grid_l +
                       curr_node(1) * sim_info.grid_l + curr_node(2);

          MPM_ASSERT(0 <= index && index < sim_info.grid_size,
                     "PARTICLE OUT OF GRID");

          grad_v += grid_attrs[index].vel_i * grad_wip.transpose();
        }

    particles[iter].F = (MT::Identity() + dt * grad_v) * F;

    // @MetaRu some trick from ZIRAN ? but it works and really stable.
    particles[iter].J = (1 + dt * grad_v.trace()) * J;

    MPM_ASSERT(grad_v == grad_v, "FATAL GRAD_VELOCITY OCCURED!");
    if (plasticity) {
      plasticity->projectStrain(particles[iter]);
    }

    if (J < 0) {
      MPM_WARN(
          "particles[{}]'s J = determinat(F) is negative!\n{}, determinant: {}\n"
          "original F:\n{}, determinant: {}\n"
          "grad_v:\n {}, determinant: {}\n"
          "J: {}",
          iter, particles[iter].F, particles[iter].F.determinant(), F,
          F.determinant(), grad_v, grad_v.determinant(), J);

      // for (int i = 0; i < 3; i++)
      //   for (int j = 0; j < 3; j++)
      //     for (int k = 0; k < 3; k++) {
      //       VINT curr_node = base_node + VINT(i, j, k);
      //       VT grad_wip{dwp(i, 0) * wp(j, 1) * wp(k, 2) * inv_h,
      //                         wp(i, 0) * dwp(j, 1) * wp(k, 2) * inv_h,
      //                         wp(i, 0) * wp(j, 1) * dwp(k, 2) * inv_h};

      //       auto index = curr_node(0) * sim_info.grid_h * sim_info.grid_l +
      //                    curr_node(1) * sim_info.grid_l + curr_node(2);
      //       weight += grid_attrs[index].vel_i * grad_wip.transpose();
      //       MPM_ERROR("check neighbor grid[{}]:\n"
      //                 "\tvel_i: {}\n"
      //                 "\tvel_in: {}\n"
      //                 "\tmass_i: {}\n"
      //                 "\tforce_i: {}\n"
      //                 "\tX_i: {}",
      //                 index, grid_attrs[index].vel_i.transpose(),
      //                 grid_attrs[index].vel_in.transpose(),
      //                 grid_attrs[index].mass_i,
      //                 grid_attrs[index].force_i.transpose(),
      //                 grid_attrs[index].Xi.transpose());
      //     }
      // assert(false);
    }
  });
  // MPM_INFO("particles[0]'s F:\n{}", particles[0].F);
}

void MPM_Simulator::transfer_G2P() {
  // MPM_SCOPED_PROFILE_FUNCTION();
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int iter) {
    // particle position in grid space
    VT particle_pos = particles[iter].pos_p;
    auto inv_h = 1.0f / sim_info.h;
    auto [base_node, wp, dwp] = quatratic_interpolation(particle_pos * inv_h);

    VT v_pic = VT::Zero();
    VT v_flip = particles[iter].vel_p;
    particles[iter].Bp = MT::Zero();

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++) {
          VINT curr_node = base_node + VINT(i, j, k);
          auto wijk = wp(i, 0) * wp(j, 1) * wp(k, 2);
          auto index = curr_node(0) * sim_info.grid_h * sim_info.grid_l +
                       curr_node(1) * sim_info.grid_l + curr_node(2);

          MPM_ASSERT(0 <= index && index < sim_info.grid_size,
                     "PARTICLE OUT OF GRID");

          v_pic += wijk * grid_attrs[index].vel_i;
          v_flip += wijk * (grid_attrs[index].vel_i - grid_attrs[index].vel_in);
          particles[iter].Bp +=
              wijk * grid_attrs[index].vel_i *
              (curr_node.cast<T>() - particle_pos * inv_h).transpose();
        }

    switch (transfer_scheme) {
    case TransferScheme::APIC:
      particles[iter].vel_p = v_pic;
      break;
    case TransferScheme::FLIP99:
    case TransferScheme::FLIP95:
      particles[iter].vel_p =
          (1 - sim_info.alpha) * v_pic + sim_info.alpha * v_flip;
    }
  });
}

void MPM_Simulator::advection(T dt) {
  // MPM_SCOPED_PROFILE_FUNCTION();
  sim_info.max_velocity = tbb::parallel_reduce(
      tbb::blocked_range<Particle *>(particles,
                                     particles + sim_info.particle_size),
      0.0f,
      [&](const tbb::blocked_range<Particle *> &r, T max_vel) -> T {
        for (auto iter = r.begin(); iter != r.end(); ++iter) {
          iter->pos_p += dt * iter->vel_p;
          max_vel = std::max(max_vel, iter->vel_p.norm());
        }
        return max_vel;
      },
      [&](T x, T y) { return std::max(x, y); });
}

void MPM_Simulator::solve_grid_collision() {
  tbb::parallel_for(0, (int)active_nodes.size(), [&](int i) {
    int index = active_nodes[i];
    for (auto &coll : colls) {
      coll.solve_collision(grid_attrs[index].Xi.cast<T>() * sim_info.h,
                           grid_attrs[index].vel_i);
    }
  });
}

void MPM_Simulator::solve_particle_collision() {
  tbb::parallel_for(0, (int)sim_info.particle_size, [&](int i) {
    for (auto &coll : colls) {
      coll.solve_collision(particles[i].pos_p, particles[i].vel_p);
    }
  });
}

void MPM_Simulator::solve_grid_boundary(int thickness) {
  // MPM_SCOPED_PROFILE_FUNCTION();
  // Sticky boundary
  auto [W, H, L] = std::tie(sim_info.grid_w, sim_info.grid_h, sim_info.grid_l);
  // check x-axis bound
  for (int i = 0; i < thickness; i++) {
    for (int j = 0; j < H; j++) {
      for (int k = 0; k < L; k++) {
        int index1 = i * H * L + j * L + k;
        int index2 = (W - i - 1) * H * L + j * L + k;
        if (grid_attrs[index1].vel_i[0] < 0) {
          grid_attrs[index1].vel_i[0] = 0.0f;
        }
        if (grid_attrs[index2].vel_i[0] > 0) {
          grid_attrs[index2].vel_i[0] = 0.0f;
        }
      }
    }
  }
  // check y-axis bound
  for (int i = 0; i < W; i++) {
    for (int j = 0; j < thickness; j++) {
      for (int k = 0; k < L; k++) {
        int index1 = i * H * L + j * L + k;
        int index2 = i * H * L + (H - j - 1) * L + k;
        if (grid_attrs[index1].vel_i[1] < 0) {
          grid_attrs[index1].vel_i[1] = 0.0f;
        }
        if (grid_attrs[index2].vel_i[1] > 0) {
          grid_attrs[index2].vel_i[1] = 0.0f;
        }
      }
    }
  }
  // check z-axis bound
  for (int i = 0; i < W; i++) {
    for (int j = 0; j < H; j++) {
      for (int k = 0; k < thickness; k++) {
        int index1 = i * H * L + j * L + k;
        int index2 = i * H * L + j * L + (L - k - 1);
        if (grid_attrs[index1].vel_i[2] < 0) {
          grid_attrs[index1].vel_i[2] = 0.0f;
        }
        if (grid_attrs[index2].vel_i[2] > 0) {
          grid_attrs[index2].vel_i[2] = 0.0f;
        }
      }
    }
  }
}

} // namespace mpm