#pragma once

#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace mpm {

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

} // namespace mpm

#ifndef MPM_NO_DEBUG

// Client log macros
#define MPM_FATAL(...) ::mpm::MPMLog::get_logger()->fatal(__VA_ARGS__)
#define MPM_ERROR(...) ::mpm::MPMLog::get_logger()->error(__VA_ARGS__)
#define MPM_WARN(...) ::mpm::MPMLog::get_logger()->warn(__VA_ARGS__)
#define MPM_INFO(...) ::mpm::MPMLog::get_logger()->info(__VA_ARGS__)
#define MPM_TRACE(...) ::mpm::MPMLog::get_logger()->trace(__VA_ARGS__)

#define MPM_ASSERT(condition, ...)                                             \
  do {                                                                         \
    if (!(condition)) {                                                        \
      MPM_ERROR(__VA_ARGS__);                                                  \
      assert(condition);                                                       \
    }                                                                          \
  } while (false)

#else

#define MPM_FATAL(...)
#define MPM_ERROR(...)
#define MPM_WARN(...)
#define MPM_INFO(...)
#define MPM_TRACE(...)

#define MPM_ASSERT(condition, ...)

#endif