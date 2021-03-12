#pragma once
#include "MPM/mpm_defs.h"

namespace mpm {

template <class Tensor> std::string make_string(const Tensor &v);

bool read_particles(const std::string &model_path,
                    std::vector<Vector3f> &positions);
bool write_particles(const std::string &write_path,
                     const std::vector<Vector3f> &positions);

inline Vector3f calc_quadratic(float o, float x);

inline Vector3f calc_quadratic_grad(float o, float x);
// under gridspace coords
std::tuple<Vector3i, Matrix3f, Matrix3f>
quatratic_interpolation(const Vector3f &particle_pos);

// define logger
class MPMLog {
public:
  static void init();
  MPMLog() = default;
  virtual ~MPMLog() = default;

  inline static const std::shared_ptr<spdlog::logger> &get_logger() {
    return s_logger;
  }

private:
  static std::shared_ptr<spdlog::logger> s_logger;
};

class MPMProfiler {
public:
  MPMProfiler(const std::string &tag);
  virtual ~MPMProfiler();

private:
  std::string tag;
  std::chrono::high_resolution_clock::time_point start;
};

#ifndef MPM_NO_DEBUG

// Detect Platform
#if defined(_WIN32)
/* Windows x86 or x64  */
#define MPM_PLATFORM_WINDOWS
#elif defined(__linux__)
#define MPM_PLATFORM_LINUX
#endif

// Client log macros
#define MPM_FATAL(...) MPMLog::get_logger()->fatal(__VA_ARGS__)
#define MPM_ERROR(...) MPMLog::get_logger()->error(__VA_ARGS__)
#define MPM_WARN(...) MPMLog::get_logger()->warn(__VA_ARGS__)
#define MPM_INFO(...) MPMLog::get_logger()->info(__VA_ARGS__)
#define MPM_TRACE(...) MPMLog::get_logger()->trace(__VA_ARGS__)

#define MPM_ASSERT(condition, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      mpm::MPM_ERROR(__VA_ARGS__);                                             \
      assert(condition);                                                       \
    }                                                                          \
  } while (false)

#define MPM_FUNCTION_SIG __func__
#define MPM_PROFILE(tag) MPMProfiler timer##__LINE__(tag)
#define MPM_PROFILE_FUNCTION() MPM_PROFILE(MPM_FUNCTION_SIG)

// define debugbreak tools
#if defined(MPM_PLATFORM_WINDOWS)
#define MPM_DEBUGBREAK() __debugbreak()
#elif defined(MPM_PLATFORM_LINUX)
#include <signal.h>
#define MPM_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet"
#endif

#else

#define MPM_FATAL(...)
#define MPM_ERROR(...)
#define MPM_WARN(...)
#define MPM_INFO(...)
#define MPM_TRACE(...)

#define MPM_ASSERT(...)

#define MPM_FUNCTION_SIG
#define MPM_PROFILE(tag)
#define MPM_PROFILE_FUNCTION()

#define MPM_DEBUGBREAK()

#endif
} // namespace mpm