#pragma once

#include "MPM/base.h"

namespace mpm {

inline Vector3f calc_quadratic(float o, float x);

inline Vector3f calc_quadratic_grad(float o, float x);
// under gridspace coords
std::tuple<Vector3i, Matrix3f, Matrix3f>
quatratic_interpolation(const Vector3f &particle_pos);
} // namespace mpm