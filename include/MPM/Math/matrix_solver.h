#pragma once

#include "MPM/base.h"

namespace mpm {
// Singular Value Decomposition

void SVDSolver(MT &F, MT &U, MT &Sigma, MT &V);
void SVDSolverDiagonal(MT &F, MT &U, VT &Sigma, MT &V);

inline VT hatOfMatrix(const MT &A) { return VT(A(0, 0), A(1, 1), A(2, 2)); }

inline MT vectorToMatrix(const VT &V) {
  MT A = MT::Identity();
  A(0, 0) = V[0];
  A(1, 1) = V[1];
  A(2, 2) = V[2];
  return A;
}

} // namespace mpm