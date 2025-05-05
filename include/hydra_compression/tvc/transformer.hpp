#pragma once
#include <vector>
#include <string>
#include <unordered_map>

namespace hydra { namespace compression {

class transformer {
public:
    std::vector<std::string> predict_masked_tokens(
        const std::vector<std::string>& tokens,
        const std::unordered_map<std::string, double>& context_probs
    );
};

}} // namespace hydra::compression
