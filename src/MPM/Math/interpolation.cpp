#include "MPM/Math/interpolation.h"

namespace mpm {

inline VT calc_quadratic(T o, T x) {
  // +-(o)------(o+1)--(x)--(o+2)-+
  T d0 = x - o;
  T d1 = d0 - 1;
  T d2 = 1 - d1;

  return {T(0.5) * T(1.5 - d0) * T(1.5 - d0), T(0.75) - d1 * d1,
          T(0.5) * T(1.5 - d2) * T(1.5 - d2)};
}

inline VT calc_quadratic_grad(T o, T x) {
  T d0 = x - o;
  T d1 = d0 - 1;
  T d2 = 1 - d1;

  return {d0 - T(1.5), -2 * d1, T(1.5) - d2};
}

// under gridspace coords
std::tuple<VINT, MT, MT> quatratic_interpolation(const VT &particle_pos) {
  VINT base_node = floor(particle_pos.array() - 0.5).cast<int>();
  MT wp, dwp;

  // note: load by columns
  wp << calc_quadratic(base_node(0), particle_pos(0)),
      calc_quadratic(base_node(1), particle_pos(1)),
      calc_quadratic(base_node(2), particle_pos(2));

  dwp << calc_quadratic_grad(base_node(0), particle_pos(0)),
      calc_quadratic_grad(base_node(1), particle_pos(1)),
      calc_quadratic_grad(base_node(2), particle_pos(2));

  return {base_node, wp, dwp};
}
} // namespace mpm