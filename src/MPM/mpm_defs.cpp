#include "MPM/mpm_defs.h"

namespace mpm {

Matrix3f neohookean_piola(float E, float nu, const Matrix3f& F) {
  auto mu = 0.5f * E / (1.0f + nu);
  auto lambda = E * nu / (1.0f + nu) / (1.0f - 2.0f * nu);
  auto J = F.determinant();
  Matrix3f piola = mu * (F - F.transpose().inverse()) +
                   lambda * log(J) * F.transpose().inverse();
  return piola;
}

std::shared_ptr<spdlog::logger> MPMLog::s_logger;
void MPMLog::init() {
  s_logger = spdlog::stdout_color_mt("MPM");
  s_logger->set_level(spdlog::level::level_enum::trace);
}

MPMProfiler::MPMProfiler(const std::string& tag) : tag(tag) {
  start = std::chrono::high_resolution_clock::now();
}

MPMProfiler::~MPMProfiler() {
  auto end = std::chrono::high_resolution_clock::now();

  MPM_TRACE("[profiler] {} cost {} ms", tag,
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                    .count() /
                1000.0f);
}

}  // namespace mpm