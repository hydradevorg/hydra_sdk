#pragma once
#include <vector>
#include <string>

namespace hydra { namespace compression {

class decoder {
public:
    std::string reconstruct_frame(const std::vector<std::string>& tokens, int frame_index = 0);
};

}} // namespace hydra::compression
