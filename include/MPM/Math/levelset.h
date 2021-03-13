#pragma once

#include "MPM/base.h"

namespace mpm {

class MPM_LevelSet {
public:
  virtual ~MPM_LevelSet() = default;
  virtual bool inside(const Vector3f &x) const = 0;
  virtual float signed_distance(const Vector3f &x) const = 0;
  virtual Vector3f normal(const Vector3f &x) const = 0;
};

class HalfPlane_LevelSet : public MPM_LevelSet {
public:
  Vector3f direction;
  Vector3f origin;

  HalfPlane_LevelSet(const Vector3f &direction, const Vector3f &origin);
  ~HalfPlane_LevelSet() = default;

  virtual bool inside(const Vector3f &x) const override;
  virtual float signed_distance(const Vector3f &x) const override;
  virtual Vector3f normal(const Vector3f &x) const override;
};

class Sphere_LevelSet : public MPM_LevelSet {
public:
  Vector3f center;
  float radius;

  Sphere_LevelSet(const Vector3f &center, float radius);
  ~Sphere_LevelSet() = default;

  virtual bool inside(const Vector3f &x) const override;
  virtual float signed_distance(const Vector3f &x) const override;
  virtual Vector3f normal(const Vector3f &x) const override;
};

} // namespace mpm