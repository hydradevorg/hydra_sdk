#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace hydra { namespace math {

/**
 * @brief Implementation of Huffman coding for data compression
 *
 * Huffman coding is a lossless data compression algorithm that assigns variable-length codes
 * to input characters based on their frequencies. The most frequent characters are assigned
 * the shortest codes, resulting in shorter encoded messages.
 */
class HuffmanCodec {
public:
    /** Map of characters to their frequencies in the input */
    using FrequencyMap = std::unordered_map<char, int>;
    /** Map of characters to their binary codes */
    using CodeMap = std::unordered_map<char, std::string>;

    /**
     * @brief Constructs a Huffman codec from an input string
     *
     * Analyzes the input string to determine character frequencies and builds
     * the Huffman tree and code map.
     *
     * @param input The input string to analyze for building the Huffman tree
     */
    explicit HuffmanCodec(const std::string& input);

    /**
     * @brief Encodes a string using the Huffman codes
     *
     * @param input The string to encode
     * @return The encoded binary string (as a sequence of '0' and '1' characters)
     * @throws std::invalid_argument if the input contains characters not in the original training data
     */
    std::string encode(const std::string& input) const;
    
    /**
     * @brief Decodes a binary string using the Huffman tree
     *
     * @param binary The binary string to decode (a sequence of '0' and '1' characters)
     * @return The decoded string
     * @throws std::invalid_argument if the binary string is not a valid Huffman encoding
     */
    std::string decode(const std::string& binary) const;
    
    /**
     * @brief Gets the mapping of characters to their Huffman codes
     *
     * @return A reference to the internal code map
     */
    const CodeMap& getCodeMap() const;

private:
    /**
     * @brief Node in the Huffman tree
     *
     * Each node contains a character, its frequency, and pointers to left and right children.
     * Leaf nodes represent actual characters in the input.
     */
    struct Node {
        char ch;      ///< Character (only meaningful for leaf nodes)
        int freq;     ///< Frequency of the character or subtree
        std::shared_ptr<Node> left;   ///< Left child (represents '0' in the code)
        std::shared_ptr<Node> right;  ///< Right child (represents '1' in the code)
        
        /**
         * @brief Checks if this node is a leaf node
         * @return true if the node is a leaf (has no children), false otherwise
         */
        bool isLeaf() const { return !left && !right; }
    };

    /**
     * @brief Comparator for Node objects in the priority queue
     *
     * Used to build the Huffman tree by always selecting the nodes with lowest frequency first.
     */
    struct NodeCmp {
        /**
         * @brief Compares two nodes based on their frequencies
         * @param a First node
         * @param b Second node
         * @return true if a's frequency is greater than b's (for min-heap behavior)
         */
        bool operator()(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b) const {
            return a->freq > b->freq;
        }
    };

    std::shared_ptr<Node> root;  ///< Root of the Huffman tree
    CodeMap codeMap;             ///< Mapping of characters to their binary codes

    /**
     * @brief Builds the Huffman tree from character frequencies
     * @param freqMap Map of characters to their frequencies
     */
    void buildTree(const FrequencyMap& freqMap);
    
    /**
     * @brief Recursively builds the code map by traversing the Huffman tree
     * @param node Current node in the traversal
     * @param prefix Current binary code prefix
     */
    void buildCodeMap(const std::shared_ptr<Node>& node, const std::string& prefix);
};

}} // namespace hydra::math
