#pragma once

#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <string>
#include <algorithm>

namespace hydra {
namespace compression {

// Simple HuffmanTree implementation for compression module
template<typename T>
class HuffmanTree {
private:
    struct Node {
        T value;
        double frequency;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        
        Node(T val, double freq) : value(val), frequency(freq), left(nullptr), right(nullptr) {}
        Node(double freq, std::shared_ptr<Node> l, std::shared_ptr<Node> r) 
            : frequency(freq), left(l), right(r) {}
        
        bool isLeaf() const { return !left && !right; }
    };
    
    std::shared_ptr<Node> root;
    std::map<T, std::string> codes;
    
    void generateCodes(std::shared_ptr<Node> node, const std::string& code) {
        if (!node) return;
        
        if (node->isLeaf()) {
            codes[node->value] = code;
            return;
        }
        
        generateCodes(node->left, code + "0");
        generateCodes(node->right, code + "1");
    }
    
public:
    HuffmanTree(const std::vector<std::pair<T, double>>& frequencies) {
        // Create a priority queue to store nodes
        auto compare = [](std::shared_ptr<Node> a, std::shared_ptr<Node> b) {
            return a->frequency > b->frequency;
        };
        
        std::priority_queue<std::shared_ptr<Node>, 
                           std::vector<std::shared_ptr<Node>>, 
                           decltype(compare)> pq(compare);
        
        // Create leaf nodes and add them to the priority queue
        for (const auto& pair : frequencies) {
            pq.push(std::make_shared<Node>(pair.first, pair.second));
        }
        
        // Build the Huffman tree
        while (pq.size() > 1) {
            auto left = pq.top(); pq.pop();
            auto right = pq.top(); pq.pop();
            
            auto parent = std::make_shared<Node>(
                left->frequency + right->frequency, left, right);
            pq.push(parent);
        }
        
        // The root is the only node left in the queue
        if (!pq.empty()) {
            root = pq.top();
            // Generate codes
            generateCodes(root, "");
        }
    }
    
    std::map<T, std::string> getCodes() const {
        return codes;
    }
};

} // namespace compression
} // namespace hydra
