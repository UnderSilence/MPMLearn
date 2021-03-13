#include "MPM/Utils/debug.h"
#include "MPM/mpm_pch.h"

namespace mpm {
template <class Tensor> std::string make_string(const Tensor &v) {
  std::string output;
  for (int i = 0; i < v.rows(); i++) {
    for (int j = 0; j < v.cols(); j++) {
      output.append(std::to_string(v(i, j)) + " ");
    }
    output.append("\n");
  }
  return output;
}

template <> std::string make_string<Vector3f>(const Vector3f &v);
template <> std::string make_string<Matrix3f>(const Matrix3f &v);

} // namespace mpm