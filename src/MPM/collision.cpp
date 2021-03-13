#include "MPM/collision.h"

namespace mpm {

MPM_Collision::MPM_Collision(const std::shared_ptr<MPM_LevelSet> &levelset,
                             CollisionType coll_type, float fraction)
    : levelset(levelset), coll_type(coll_type), friction(friction) {}

void MPM_Collision::solve_collision(const Vector3f& xi, Vector3f& vi) {
    if(levelset->inside(xi)) {
        if(coll_type == STICKY) {
            vi = Vector3f::Zero();
        } else {
            auto n = levelset->normal(xi);
            float vi_dot_n = vi.dot(n);
            if(vi_dot_n < 0) {
                vi -= vi_dot_n * n;
                if(friction != 0) {
                    if(vi.norm() > -friction * vi_dot_n) {
                        vi += friction * vi_dot_n * vi.normalized();
                    } else {
                        vi = Vector3f::Zero();
                    }
                } 
            }
        }
    }
}

} // namespace mpm