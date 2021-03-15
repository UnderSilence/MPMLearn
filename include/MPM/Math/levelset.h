#pragma once

#include "MPM/base.h"

namespace mpm {

class MPM_LevelSet {
public:
  virtual ~MPM_LevelSet() = default;
  virtual bool inside(const VT &x) const = 0;
  virtual T signed_distance(const VT &x) const = 0;
  virtual VT normal(const VT &x) const = 0;
};

class HalfPlane_LevelSet : public MPM_LevelSet {
public:
  VT direction;
  VT origin;

  HalfPlane_LevelSet(const VT &direction, const VT &origin);
  ~HalfPlane_LevelSet() = default;

  virtual bool inside(const VT &x) const override;
  virtual T signed_distance(const VT &x) const override;
  virtual VT normal(const VT &x) const override;
};

class Sphere_LevelSet : public MPM_LevelSet {
public:
  VT center;
  T radius;

  Sphere_LevelSet(const VT &center, T radius);
  ~Sphere_LevelSet() = default;

  virtual bool inside(const VT &x) const override;
  virtual T signed_distance(const VT &x) const override;
  virtual VT normal(const VT &x) const override;
};

} // namespace mpm