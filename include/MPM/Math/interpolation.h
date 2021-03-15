#pragma once

#include "MPM/base.h"

namespace mpm {

inline VT calc_quadratic(T o, T x);

inline VT calc_quadratic_grad(T o, T x);
// under gridspace coords
std::tuple<VINT, MT, MT> quatratic_interpolation(const VT &particle_pos);
} // namespace mpm