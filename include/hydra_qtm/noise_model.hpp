#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

namespace hydra::qtm {

struct NoiseChannel {
    std::string name;
    std::vector<Eigen::MatrixXcd> kraus_ops;
};

class NoiseModel {
public:
    void add_channel(size_t qubit, const NoiseChannel& channel);
    const std::vector<NoiseChannel>& channels_for(size_t qubit) const;

private:
    std::unordered_map<size_t, std::vector<NoiseChannel>> noise_map_;
};

} // namespace hydra::qtm
