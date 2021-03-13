#include "MPM/Utils/logger.h"

namespace mpm {

std::shared_ptr<spdlog::logger> MPMLog::s_logger;
void MPMLog::init() {
  s_logger = spdlog::stdout_color_mt("MPM");
  s_logger->set_pattern("[%^%l%$][%n]%v");
  s_logger->set_level(spdlog::level::level_enum::trace);
}

} // namespace mpm