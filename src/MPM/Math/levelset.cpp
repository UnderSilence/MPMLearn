#include "MPM/Math/levelset.h"

namespace mpm {

HalfPlane_LevelSet::HalfPlane_LevelSet(const VT &direction, const VT &origin)
    : direction(direction), origin(origin) {
  this->direction.normalize();
}

bool HalfPlane_LevelSet::inside(const VT &x) const {
  return signed_distance(x) <= 0;
}

T HalfPlane_LevelSet::signed_distance(const VT &x) const {
  return direction.dot(x - origin);
}

VT HalfPlane_LevelSet::normal(const VT &x) const { return direction; }

Sphere_LevelSet::Sphere_LevelSet(const VT &center, T radius)
    : center(center), radius(radius) {}

bool Sphere_LevelSet::inside(const VT &x) const {
  return (x - center).squaredNorm() <= radius * radius;
};

T Sphere_LevelSet::signed_distance(const VT &x) const {
  return (x - center).norm() - radius;
};

VT Sphere_LevelSet::normal(const VT &x) const { return x - center; }

} // namespace mpm