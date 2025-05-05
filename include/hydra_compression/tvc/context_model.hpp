#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace hydra { namespace compression {

class context_model {
public:
    void train(const std::vector<std::string>& tokens);
    std::unordered_map<std::string, double> get_probabilities() const;

private:
    std::unordered_map<std::string, int> freq_map;
};

}} // namespace hydra::compression
