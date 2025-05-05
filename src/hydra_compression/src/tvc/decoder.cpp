#include "hydra_compression/tvc/decoder.hpp"
#include <sstream>

namespace hydra { namespace compression {

std::string decoder::reconstruct_frame(const std::vector<std::string>& tokens, int frame_index) {
    std::ostringstream reconstructed;
    reconstructed << "[Frame " << frame_index << "] ";
    for (const auto& tok : tokens) {
        reconstructed << tok << ' ';
    }
    return reconstructed.str();
}

}} // namespace hydra::compression
