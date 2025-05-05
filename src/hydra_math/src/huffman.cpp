#include "hydra_math/huffman.hpp"
#include <queue>
#include <sstream>

namespace hydra { namespace math {

HuffmanCodec::HuffmanCodec(const std::string& input) {
    FrequencyMap freq;
    for (char c : input) ++freq[c];
    buildTree(freq);
    buildCodeMap(root, "");
}

void HuffmanCodec::buildTree(const FrequencyMap& freqMap) {
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, NodeCmp> pq;

    for (const auto& [ch, f] : freqMap) {
        pq.push(std::make_shared<Node>(Node{ch, f, nullptr, nullptr}));
    }

    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();
        auto merged = std::make_shared<Node>(Node{'\0', left->freq + right->freq, left, right});
        pq.push(merged);
    }

    root = pq.top();
}

void HuffmanCodec::buildCodeMap(const std::shared_ptr<Node>& node, const std::string& prefix) {
    if (!node) return;
    if (node->isLeaf()) {
        codeMap[node->ch] = prefix.empty() ? "0" : prefix;
    } else {
        buildCodeMap(node->left, prefix + "0");
        buildCodeMap(node->right, prefix + "1");
    }
}

std::string HuffmanCodec::encode(const std::string& input) const {
    std::ostringstream oss;
    for (char c : input) {
        oss << codeMap.at(c);
    }
    return oss.str();
}

std::string HuffmanCodec::decode(const std::string& binary) const {
    std::ostringstream oss;
    auto node = root;
    for (char bit : binary) {
        node = (bit == '0') ? node->left : node->right;
        if (node->isLeaf()) {
            oss << node->ch;
            node = root;
        }
    }
    return oss.str();
}

const HuffmanCodec::CodeMap& HuffmanCodec::getCodeMap() const {
    return codeMap;
}

}} // namespace hydra::math
