#include "hydra_compression/tvc/quantizer.hpp"
#include <unordered_map>

namespace hydra { namespace compression {

std::vector<int> quantizer::quantize(const std::vector<std::string>& tokens, int levels) {
    std::unordered_map<std::string, int> token_to_id;
    std::vector<int> indices;
    int id = 0;

    for (const auto& tok : tokens) {
        if (token_to_id.count(tok) == 0) {
            token_to_id[tok] = id++ % levels;
        }
        indices.push_back(token_to_id[tok]);
    }
    return indices;
}

std::vector<std::string> quantizer::dequantize(const std::vector<int>& indices, int levels) {
    std::vector<std::string> tokens;
    for (int idx : indices) {
        tokens.push_back("token_" + std::to_string(idx % levels));
    }
    return tokens;
}

}} // namespace hydra::compression
