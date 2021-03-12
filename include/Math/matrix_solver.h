#pragma once
#include "Eigen/Eigen"

using namespace Eigen;

// Singular Value Decomposition
void SVDSolverFloat(Matrix3f &F, Matrix3f &U, Matrix3f &Sigma, Matrix3f &V) {
  Eigen::JacobiSVD<Eigen::Matrix3f> svd(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Matrix3f tempU = svd.matrixU();
  Matrix3f tempV = svd.matrixV();

  Vector3f singVals = svd.singularValues();
  Matrix3f tempSigma = Matrix3f::Zero();
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
    double tempRecord = tempSigma(0, 0);
    tempSigma(0, 0) = tempSigma(1, 1);
    tempSigma(1, 1) = tempRecord;
  }

  U = tempU;
  V = tempV;
  Sigma = tempSigma;
}

inline Vector3f hatOfMatrix(Matrix3f A) {
  return Vector3f(A(0, 0), A(1, 1), A(2, 2));
}

inline Matrix3f vectorToMatrix(Vector3f V) { 
  Matrix3f A = Matrix3f::Identity();
  A(0, 0) = V[0];
  A(1, 1) = V[1];
  A(2, 2) = V[2];
  return A;
}