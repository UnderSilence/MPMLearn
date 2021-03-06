#include "MPM/Math/matrix_solver.h"

namespace mpm {

void SVDSolver(MT &F, MT &U, MT &Sigma, MT &V) {
  Eigen::JacobiSVD<MT> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
  MT tempU = svd.matrixU();
  MT tempV = svd.matrixV();

  VT singVals = svd.singularValues();
  MT tempSigma = MT::Zero();
  tempSigma(0, 0) = singVals(0);
  tempSigma(1, 1) = singVals(1);
  tempSigma(2, 2) = singVals(2);

  // sorting
  if (tempU.determinant() < 0) {
    tempU(0, 2) *= -1;
    tempU(1, 2) *= -1;
    tempU(2, 2) *= -1;
    tempSigma(2, 2) *= -1;
  }

  if (tempV.determinant() < 0) {
    tempV(0, 2) *= -1;
    tempV(1, 2) *= -1;
    tempV(2, 2) *= -1;
    tempSigma(2, 2) *= -1;
  }

  if (tempSigma(0, 0) < tempSigma(1, 1)) {
    T tempRecord = tempSigma(0, 0);
    tempSigma(0, 0) = tempSigma(1, 1);
    tempSigma(1, 1) = tempRecord;
  }

  U = tempU;
  V = tempV;
  Sigma = tempSigma;
}

// Singular Value Decomposition
void SVDSolverDiagonal(MT &F, MT &U, VT &Sigma, MT &V) {
  Eigen::JacobiSVD<MT> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
  MT tempU = svd.matrixU();
  MT tempV = svd.matrixV();

  VT singVals = svd.singularValues();

  // sorting
  if (tempU.determinant() < 0) {
    tempU(0, 2) *= -1;
    tempU(1, 2) *= -1;
    tempU(2, 2) *= -1;
    singVals(2) *= -1;
  }

  if (tempV.determinant() < 0) {
    tempV(0, 2) *= -1;
    tempV(1, 2) *= -1;
    tempV(2, 2) *= -1;
    singVals(2) *= -1;
  }

  if (singVals(0) < singVals(1)) {
    T tempRecord = singVals(0);
    singVals(0) = singVals(1);
    singVals(1) = tempRecord;
  }

  U = tempU;
  V = tempV;
  Sigma = singVals;
}

} // namespace mpm