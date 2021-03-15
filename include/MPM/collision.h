#pragma once

#include "MPM/Math/levelset.h"
#include "MPM/base.h"
#include <memory>

namespace mpm {
class MPM_Collision {
public:
  enum CollisionType { STICKY, SLIP };

  MPM_Collision(const std::shared_ptr<MPM_LevelSet> &levelset,
                CollisionType coll_type = SLIP, float fraction = 0.0f);

  virtual ~MPM_Collision() = default;

  virtual void solve_collision(const Vector3f& xi, Vector3f& vi);

private:
  std::shared_ptr<MPM_LevelSet> levelset;
  CollisionType coll_type;
  float friction;
};
} // namespace mpm