

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
  float h = 0.02f;
  auto W = static_cast<int>(1.0f / h + 1);
  auto H = static_cast<int>(1.0f / h + 1);
  auto L = static_cast<int>(1.0f / h + 1);
  Vector3f pos(0.648932f, 0.121521f, 0.265484f);
  auto [base_node, wp, dwp] = mpm::quatratic_interpolation(pos / h);
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      for (int k = 0; k < 3; k++) {
        float wi = wp(i, 0), wj = wp(j, 1), wk = wp(k, 2);
        float dwi = dwp(i, 0), dwj = dwp(j, 1), dwk = dwp(k, 2);
        float wijk = wi * wj * wk;
        Vector3i curr_node = base_node + Vector3i(i, j, k);

        int index = curr_node(0) * H * L + curr_node(1) * L + curr_node(2);
        Vector3f grad_wp(dwi * wj * wk / h, wi * dwj * wk / h,
                         wi * wj * dwk / h);

        MPM_INFO("offset: {}", Vector3i(i, j, k).transpose());
        MPM_INFO("weight_ijk: {}", wijk);
        MPM_INFO("grad_wp: {}", grad_wp.transpose());
      }
  MPM_INFO("{} end", __func__);
}

int main() {
  // tbb::task_scheduler_init init(1);
  // initialize logger
  mpm::MPMLog::init();
  // quatratic_test();
  auto sim = std::make_shared<mpm::MPM_Simulator>();

  auto mtl_jello = MPM_Material(50.0f, 0.3f, 10.0f, 1.0f);
  auto mtl_water = MPM_Material(50.0f, 0.40f, 0.001f, 1.0f);

  auto cm_solid = std::make_shared<mpm::NeoHookean_Piola>();
  auto cm_fluid = std::make_shared<mpm::NeoHookean_Fluid>();
  auto cm_fluid_1 = std::make_shared<mpm::QuatraticVolumePenalty>();
  auto cm_fluid_2 = std::make_shared<mpm::CDMPM_Fluid>();

  auto plas_snow = std::make_shared<mpm::Snow>();

  sim->clear_simulation();
  Vector3f gravity{0.0f, -9.8f, 0.0f};
  Vector3f area{1.0f, 1.0f, 1.0f};
  // Vector3f velocity{-2.5f, 0.5f, -0.3f};
  Vector3f velocity{0.0f, 0.0f, 0.0f};
  float h = 0.02f;

  auto wall_left = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.1f, 0.0f, 0.0f)));
  auto wall_right = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          Vector3f(-1.0f, 0.0f, 0.0f), Vector3f(0.9f, 0.0f, 0.0f)));
  auto wall_forward = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 0.0f, 0.1f)));
  auto wall_back = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 0.0f, 0.9f)));
  auto wall_bottom = mpm::MPM_Collision(std::make_shared<mpm::HalfPlane_LevelSet>(
          Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0.0f, 0.01f, 0.0f)));


  sim->mpm_initialize(gravity, area, h);
  sim->set_constitutive_model(cm_fluid_2);
  sim->add_collision(wall_left, wall_right, wall_back, wall_forward, wall_bottom);
  // sim->set_plasticity(plas_snow);

  sim->set_transfer_scheme(mpm::MPM_Simulator::TransferScheme::FLIP99);

  std::vector<Vector3f> positions;
  auto model_path = "../../models/dense_cube.obj";

  if (mpm::read_particles(model_path, positions)) {
    sim->add_object(positions,
                    std::vector<Vector3f>(positions.size(), velocity),
                    &mtl_water);
  } else {
    return 0;
  }

  float CFL = 0.05f;
  float max_dt = 5e-4f;
  float dt = max_dt;

  int frame_rate = 60;
  int total_frame = 200;
  float time_per_frame = 1.0f / frame_rate;
  float total_time = 0.0f;

  MPM_INFO("Simulation start, Meta Informations:\n"
           "\tframe_rate: {}\n"
           "\tmax_dt: {}\n"
           "\tparticle_size: {}",
           frame_rate, dt, positions.size());

  std::string output_dir("../../output/test/");
  write_particles(output_dir + "0.bgeo", sim->get_positions());

  for (int frame = 0; frame < total_frame;) {
    {
      MPM_SCOPED_PROFILE("frame#" + std::to_string(frame + 1));

      float mmax_vel = 0.0f;
      float curr_time = 0.0f;
      float min_dt = max_dt;
      int steps = 0;

      while (curr_time < time_per_frame) {
        dt = std::min(max_dt,
                      h * CFL / std::max(0.0001f, sim->get_max_velocity()));
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

      write_particles(output_dir + std::to_string(++frame) + ".bgeo",
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