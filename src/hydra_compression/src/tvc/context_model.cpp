#include "hydra_compression/tvc/context_model.hpp"

namespace hydra { namespace compression {

void context_model::train(const std::vector<std::string>& tokens) {
    freq_map.clear();
    for (const auto& token : tokens) {
        freq_map[token]++;
    }
}

std::unordered_map<std::string, double> context_model::get_probabilities() const {
    std::unordered_map<std::string, double> probs;
    int total = 0;
    for (const auto& [token, freq] : freq_map) {
        total += freq;
    }

    for (const auto& [token, freq] : freq_map) {
        probs[token] = static_cast<double>(freq) / total;
    }
    return probs;
}

}} // namespace hydra::compression
