#pragma once
#include <vector>
#include <string>

namespace hydra { namespace compression {

class tokenizer {
public:
    std::vector<std::string> tokenize(const std::string& frame_data);
    std::string detokenize(const std::vector<std::string>& tokens);
};

}} // namespace hydra::compression
