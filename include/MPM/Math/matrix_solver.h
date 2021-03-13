#pragma once

#include "MPM/base.h"

namespace mpm {
// Singular Value Decomposition

void SVDSolver(Matrix3f &F, Matrix3f &U, Matrix3f &Sigma, Matrix3f &V);
void SVDSolverDiagonal(Matrix3f &F, Matrix3f &U, Vector3f &Sigma, Matrix3f &V);

inline Vector3f hatOfMatrix(const Matrix3f &A) {
  return Vector3f(A(0, 0), A(1, 1), A(2, 2));
}

inline Matrix3f vectorToMatrix(const Vector3f &V) {
  Matrix3f A = Matrix3f::Identity();
  A(0, 0) = V[0];
  A(1, 1) = V[1];
  A(2, 2) = V[2];
  return A;
}

} // namespace mpm