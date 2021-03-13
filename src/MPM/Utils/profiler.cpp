#include "MPM/Utils/profiler.h"
#include "MPM/Utils/logger.h"

namespace mpm {

MPMScopedProfiler::MPMScopedProfiler(const std::string &tag) : tag(tag) {
  start = std::chrono::high_resolution_clock::now();
}

MPMScopedProfiler::~MPMScopedProfiler() {
  auto end = std::chrono::high_resolution_clock::now();
  MPM_TRACE("[profiler] {} cost {} ms", tag,
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                    .count() /
                1000.0f);
}
} // namespace mpm