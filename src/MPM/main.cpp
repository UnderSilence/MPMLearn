

#include <tbb/task_scheduler_init.h>

#include "MPM/Math/interpolation.h"
#include "MPM/Physics/constitutive_model.h"
#include "MPM/Physics/plasticity.h"
#include "MPM/Utils/io.h"
#include "MPM/Utils/logger.h"
#include "MPM/Utils/profiler.h"
#include "MPM/collision.h"
#include "MPM/simulator.h"

using namespace std;
using namespace Eigen;
using namespace mpm;
namespace fs = std::filesystem;

void quatratic_test() {
  MPM_PROFILE_FUNCTION();
  MPM_INFO("{} start", __func__);
  T h = 0.02f;
  auto W = static_cast<int>(1.0f / h + 1);
  auto H = static_cast<int>(1.0f / h + 1);
  auto L = static_cast<int>(1.0f / h + 1);
  VT pos(0.648932f, 0.121521f, 0.265484f);
  auto [base_node, wp, dwp] = mpm::quatratic_interpolation(pos / h);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++) {
        T wi = wp(i, 0), wj = wp(j, 1), wk = wp(k, 2);
        T dwi = dwp(i, 0), dwj = dwp(j, 1), dwk = dwp(k, 2);
        T wijk = wi * wj * wk;
        VINT curr_node = base_node + VINT(i, j, k);

        int index = curr_node(0) * H * L + curr_node(1) * L + curr_node(2);
        VT grad_wp(dwi * wj * wk / h, wi * dwj * wk / h, wi * wj * dwk / h);

        MPM_INFO("offset: {}", VINT(i, j, k).transpose());
        MPM_INFO("weight_ijk: {}", wijk);
        MPM_INFO("grad_wp: {}", grad_wp.transpose());
      }
  MPM_INFO("{} end", __func__);
}

int main(int argc, char **argv) {
  // initialize logger
  mpm::MPMLog::init();
  // quatratic_test();
  auto sim = std::make_shared<mpm::MPM_Simulator>();

  auto mtl_jello = MPM_Material(50.0, 0.3, 10.0, 1.0);
  auto mtl_water = MPM_Material(5e5, 0.4, 0.01, 1000.0);

  auto cm_solid = std::make_shared<mpm::NeoHookean_Piola>();
  auto cm_fluid = std::make_shared<mpm::NeoHookean_Fluid>();
  auto cm_fluid_1 = std::make_shared<mpm::QuatraticVolumePenalty>();
  auto cm_fluid_2 = std::make_shared<mpm::CDMPM_Fluid>();

  auto plas_snow = std::make_shared<mpm::Snow>();

  sim->clear_simulation();
  VT gravity{0.0, -9.8, 0.0};
  VT area{10.0, 10.0, 10.0};
  // VT velocity{-2.5f, 0.5f, -0.3f};
  VT velocity{0.0, 0.0, 0.0};
  T h = 0.1;

  auto wall_left = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
      VT(1.0, 0.0, 0.0), VT(2, 0.0, 0.0)));
  auto wall_right =
      mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          VT(-1.0, 0.0, 0.0), VT(8.2, 0.0, 0.0)));
  auto wall_forward =
      mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          VT(0.0, 0.0, 1.0), VT(0.0, 0.0, 1.8)));
  auto wall_back = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
      VT(0.0, 0.0, -1.0), VT(0.0, 0.0, 6)));
  // auto wall_bottom =
  //     mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
  //         VT(0.0, 1.0, 0.0f), VT(0.0, 0.1, 0.0)));

  sim->mpm_initialize(gravity, area, h);
  sim->set_constitutive_model(cm_fluid_1);
  sim->add_collision(wall_left, wall_right, wall_back, wall_forward);
  // sim->set_plasticity(plas_snow);

  sim->set_transfer_scheme(mpm::MPM_Simulator::TransferScheme::APIC);

  std::vector<VT> positions;
  auto model_path = "../../models/sand.obj";

  if (mpm::read_particles(model_path, positions)) {
    sim->add_object(positions, std::vector<VT>(positions.size(), velocity),
                    &mtl_water);
  } else {
    return 0;
  }

  T CFL = 0.5;
  T max_dt = 1e-3;
  T dt = max_dt;

  int frame_rate = 30;
  int total_frame = 300;
  T time_per_frame = 1.0 / frame_rate;
  T total_time = 0.0;

  MPM_INFO("Simulation start, Meta Informations:\n"
           "\tframe_rate: {}\n"
           "\tmax_dt: {}\n"
           "\tparticle_size: {}",
           frame_rate, dt, positions.size());

  fs::path output_dir("../../output/test/");
  if (!fs::exists(output_dir)) {
    fs::create_directory(output_dir);
  }

  write_particles(output_dir.generic_string() + "0.bgeo", sim->get_positions());

  for (int frame = 0; frame < total_frame;) {
    {
      MPM_SCOPED_PROFILE("frame#" + std::to_string(frame + 1));

      T mmax_vel = 0.0f;
      T curr_time = 0.0f;
      T min_dt = max_dt;
      int steps = 0;

      while (curr_time < time_per_frame) {
        dt = std::min(max_dt,
                      h * CFL / std::max(T(0.0001), sim->get_max_velocity()));
        mmax_vel = std::max(mmax_vel, sim->get_max_velocity());

        min_dt = std::min(dt, min_dt);

        if (dt + curr_time >= time_per_frame) {
          dt = time_per_frame - curr_time;
        }
        sim->substep(dt);

        steps++;
        curr_time += dt;
        total_time += dt;
      }

      write_particles(output_dir.generic_string() + std::to_string(++frame) +
                          ".bgeo",
                      sim->get_positions());
      MPM_INFO("frame#{} info:\n"
               "\tsteps: {}\n"
               "\tmax_vel: {}\n"
               "\tmin_dt, max_dt: {}, {}\n"
               "\ttotal_time: {}",
               frame, steps, mmax_vel, min_dt, max_dt, total_time);
    }
  }
  // sim->mpm_demo(cm_fluid, "neohookean_fluids/");

  MPM_INFO("===SIMULATION FINISHED===\n");
  return 0;
}