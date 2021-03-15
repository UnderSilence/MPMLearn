#pragma once

#include "MPM/Math/levelset.h"
#include "MPM/base.h"
#include <memory>

namespace mpm {
class MPM_Collision {
public:
  enum CollisionType { STICKY, SLIP };

  MPM_Collision(const std::shared_ptr<MPM_LevelSet> &levelset,
                CollisionType coll_type = SLIP, T friction = 0);

  virtual ~MPM_Collision() = default;

  virtual void solve_collision(const VT &xi, VT &vi);

private:
  std::shared_ptr<MPM_LevelSet> levelset;
  CollisionType coll_type;
  T friction;
};

} // namespace mpm