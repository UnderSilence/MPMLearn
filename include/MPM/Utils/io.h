#

namespace MPM {

bool read_particles(const std::string &model_path,
                    std::vector<Vector3f> &positions);
bool write_particles(const std::string &write_path,
                     const std::vector<Vector3f> &positions);
} // namespace MPM