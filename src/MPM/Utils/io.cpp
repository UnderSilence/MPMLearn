#include "MPM/Utils/io.h"
#include "MPM/mpm_pch.h"
#include "Partio.h"

namespace mpm {

bool read_particles(const std::string &model_path, std::vector<VT> &positions) {
  std::ifstream input(model_path);
  std::string line;
  std::stringstream ss;
  VT pos;
  T x, y, z;

  if (input) {
    positions.clear();
    while (std::getline(input, line)) {
      if (line[0] == 'v') {
        ss.clear();
        ss.str(line.c_str() + 1);
        if constexpr (DIM == 3) {
          ss >> x >> y >> z;
          pos << x, y, z;
        } else if constexpr (DIM == 2) {
          ss >> x >> y;
          pos << x, y;
        }
        positions.push_back(pos);
      }
    }

    MPM_INFO("read in particles[size: {}] from {} SUCCESS", positions.size(),
             model_path);
    return true;
  } else {
    MPM_ERROR("model_path:{} not found", model_path);
    return false;
  }
}

bool write_particles(const std::string &write_path,
                     const std::vector<VT> &positions) {
  Partio::ParticlesDataMutable *parts = Partio::create();
  Partio::ParticleAttribute pos_attr =
      parts->addAttribute("position", Partio::VECTOR, 3);
  Partio::ParticleAttribute index_attr =
      parts->addAttribute("index", Partio::INT, 1);

  for (auto i = 0; i < positions.size(); i++) {
    int idx = parts->addParticle();
    auto *p = parts->dataWrite<Eigen::Vector3f>(pos_attr, idx);
    auto *index = parts->dataWrite<int>(index_attr, idx);

    *p = positions[i].cast<float>();
    *index = i;
  }

  Partio::write(write_path.c_str(), *parts);
  parts->release();
  return true;
}

template <typename Vec> void read_from_string(Vec &x, const char *str) {}

} // namespace mpm