#include "hydra_qtm/noise_model.hpp"
#include <unordered_map>

namespace hydra::qtm {

void NoiseModel::add_channel(size_t qubit, const NoiseChannel& channel) {
    noise_map_[qubit].push_back(channel);
}

const std::vector<NoiseChannel>& NoiseModel::channels_for(size_t qubit) const {
    static const std::vector<NoiseChannel> empty;
    auto it = noise_map_.find(qubit);
    if (it == noise_map_.end()) {
        return empty;
    }
    return it->second;
}

} // namespace hydra::qtm
