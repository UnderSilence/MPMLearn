#include "MPM/collision.h"

namespace mpm {

MPM_Collision::MPM_Collision(const std::shared_ptr<MPM_LevelSet> &levelset,
                             CollisionType coll_type, float fraction_rate)
    levelset(levelset),
    coll_type(coll_type), fraction_rate(fraction_rate) {}

void MPM_Collision::solve_grid_collision(const GridAttr &grid_attr, float dx) {}
void MPM_Collision::solve_particle_collision(const Particle &particle) {}

} // namespace mpm