#include "MPM/Utils/io.h"
#include "MPM/mpm_pch.h"
#include "Partio.h"

namespace mpm {

bool read_particles(const std::string &model_path,
                    std::vector<Vector3f> &positions) {
  std::ifstream input(model_path);
  std::string line;
  Vector3f pos;

  if (input) {
    positions.clear();
    while (std::getline(input, line)) {
      if (line[0] == 'v') {
        sscanf(line.c_str(), "v %f %f %f", &pos[0], &pos[1], &pos[2]);
        positions.push_back(pos);
      }
    }
    return true;
  } else {
    MPM_ERROR("model_path:{} not found", model_path);
    return false;
  }
}

bool write_particles(const std::string &write_path,
                     const std::vector<Vector3f> &positions) {
  Partio::ParticlesDataMutable *parts = Partio::create();
  Partio::ParticleAttribute pos_attr =
      parts->addAttribute("position", Partio::VECTOR, 3);
  Partio::ParticleAttribute index_attr =
      parts->addAttribute("index", Partio::INT, 1);

  for (auto i = 0; i < positions.size(); i++) {
    int idx = parts->addParticle();
    auto *p = parts->dataWrite<Vector3f>(pos_attr, idx);
    auto *index = parts->dataWrite<int>(index_attr, idx);

    *p = positions[i];
    *index = i;
  }

  Partio::write(write_path.c_str(), *parts);
  parts->release();
  return true;
}
} // namespace mpm