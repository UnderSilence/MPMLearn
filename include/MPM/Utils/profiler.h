#pragma once

#include <chrono>
#include <string>

namespace mpm {
class MPMScopedProfiler {
public:
  MPMScopedProfiler(const std::string &tag);
  virtual ~MPMScopedProfiler();

private:
  std::string tag;
  std::chrono::high_resolution_clock::time_point start;
};
} // namespace mpm

#ifndef MPM_NO_DEBUG

#define MPM_FUNCTION_SIG __func__
#define MPM_SCOPED_PROFILE(tag) MPMScopedProfiler timer##__LINE__(tag)
#define MPM_PROFILE_FUNCTION() MPM_SCOPED_PROFILE(MPM_FUNCTION_SIG)

#else

#define MPM_FUNCTION_SIG
#define MPM_SCOPED_PROFILE(tag)
#define MPM_PROFILE_FUNCTION()

#endif