#pragma once
#include <vector>
#include <string>

namespace hydra { namespace compression {

class quantizer {
public:
    std::vector<int> quantize(const std::vector<std::string>& tokens, int levels = 16);
    std::vector<std::string> dequantize(const std::vector<int>& indices, int levels = 16);
};

}} // namespace hydra::compression
