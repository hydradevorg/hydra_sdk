#include "hydra_compression/tvc/tokenizer.hpp"
#include <sstream>

namespace hydra { namespace compression {

std::vector<std::string> tokenizer::tokenize(const std::string& frame_data) {
    std::istringstream stream(frame_data);
    std::vector<std::string> tokens;
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string tokenizer::detokenize(const std::vector<std::string>& tokens) {
    std::ostringstream result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result << tokens[i];
        if (i + 1 < tokens.size()) result << ' ';
    }
    return result.str();
}

}} // namespace hydra::compression
