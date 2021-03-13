#pragma once

#include "MPM/Math/levelset.h"
#include "MPM/base.h"
#include <memory>

namespace mpm {
class MPM_Collision {
public:
  enum CollisionType { STICKY, SLIP };

  MPM_Collision(const std::shared_ptr<MPM_LevelSet> &levelset,
                CollisionType coll_type, float fraction_rate);

  virtual ~MPM_Collision() = default;

  virtual void solve_grid_collision(const GridAttr &grid_attr, float dx);
  virtual void solve_particle_collision(const Particle &particle);

private:
  std::shared_ptr<MPM_LevelSet> levelset;
  CollisionType coll_type;
  float fraction_rate;
};
} // namespace mpm