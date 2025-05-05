#include "hydra_compression/tvc/transformer.hpp"
#include <random>

namespace hydra { namespace compression {

std::vector<std::string> transformer::predict_masked_tokens(
    const std::vector<std::string>& tokens,
    const std::unordered_map<std::string, double>& context_probs
) {
    std::vector<std::string> output;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (const auto& token : tokens) {
        if (token == "[MASK]") {
            double max_prob = -1.0;
            std::string best_token = "";
            for (const auto& [tok, prob] : context_probs) {
                double rand_score = prob * dis(gen); // randomness injection
                if (rand_score > max_prob) {
                    max_prob = rand_score;
                    best_token = tok;
                }
            }
            output.push_back(best_token);
        } else {
            output.push_back(token);
        }
    }
    return output;
}

}} // namespace hydra::compression
