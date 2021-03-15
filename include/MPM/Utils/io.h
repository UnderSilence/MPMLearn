#pragma once

#include "MPM/base.h"
#include <string>

namespace mpm {
bool read_particles(const std::string &model_path, std::vector<VT> &positions);
bool write_particles(const std::string &write_path,
                     const std::vector<VT> &positions);

bool is_dir(const std::string &path);
bool is_file(const std::string &path);
bool mkdir(const std::string &path);
bool exists(const std::string &path);

} // namespace mpm