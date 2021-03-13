#include "MPM/Math/levelset.h"

namespace mpm {

HalfPlane_LevelSet::HalfPlane_LevelSet(const Vector3f &direction,
                                       const Vector3f &origin)
    : direction(direction), origin(origin) {
  this->direction.normalize();
}

bool HalfPlane_LevelSet::inside(const Vector3f &x) const {
  return signed_distance(x) <= 0;
}

float HalfPlane_LevelSet::signed_distance(const Vector3f &x) const {
  return direction.dot(x - origin);
}

Vector3f HalfPlane_LevelSet::normal(const Vector3f &x) const {
  return direction;
}

Sphere_LevelSet::Sphere_LevelSet(const Vector3f &center, float radius)
    : center(center), radius(radius) {}

bool Sphere_LevelSet::inside(const Vector3f &x) const {
  return (x - center).squaredNorm() <= radius * radius;
};

float Sphere_LevelSet::signed_distance(const Vector3f &x) const {
  return (x - center).norm() - radius;
};

Vector3f Sphere_LevelSet::normal(const Vector3f &x) const { return x - center; }

} // namespace mpm