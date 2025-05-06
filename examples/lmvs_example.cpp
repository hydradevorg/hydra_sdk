#include "lmvs/lmvs.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// Helper function to print a vector
void printVector(const std::vector<double>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::fixed << std::setprecision(4) << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    std::cout << "Layered Matrix and Vector System (LMVS) Example" << std::endl;
    std::cout << "===============================================" << std::endl;
    
    // Initialize LMVS system
    const size_t num_layers = 3;
    const size_t dimension = 4;
    const size_t num_nodes = 5;
    const size_t threshold = 3;
    
    lmvs::LMVS system(num_layers, dimension, num_nodes, threshold);
    
    // Create a layered vector
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},     // Layer 1
        {5.0, 6.0, 7.0, 8.0},     // Layer 2
        {9.0, 10.0, 11.0, 12.0}   // Layer 3
    };
    
    lmvs::LayeredVector vector = system.createVector(data);
    
    std::cout << "\n1. Created Layered Vector:" << std::endl;
    vector.print();
    
    // Create another layered vector
    std::vector<std::vector<double>> data2 = {
        {0.1, 0.2, 0.3, 0.4},     // Layer 1
        {0.5, 0.6, 0.7, 0.8},     // Layer 2
        {0.9, 1.0, 1.1, 1.2}      // Layer 3
    };
    
    lmvs::LayeredVector vector2 = system.createVector(data2);
    
    // Create a layered matrix from the two vectors
    lmvs::LayeredMatrix matrix = system.createMatrix(vector, vector2);
    
    std::cout << "\n2. Created Layered Matrix from two vectors:" << std::endl;
    std::cout << "   (Only showing first block for brevity)" << std::endl;
    std::cout << "   Block [0][0]:" << std::endl;
    const auto& block = matrix.getBlock(0, 0);
    for (const auto& row : block) {
        std::cout << "   ";
        printVector(row);
        std::cout << std::endl;
    }
    
    // Project the vector to a lower dimension
    const size_t output_dim = 2;
    lmvs::LayeredVector projected = system.projectVector(vector, output_dim);
    
    std::cout << "\n3. Projected Vector (from dimension " << dimension << " to " << output_dim << "):" << std::endl;
    projected.print();
    
    // Split the vector using secret sharing
    const size_t num_shares = 5;
    const size_t share_threshold = 3;
    auto shares = system.splitVector(vector, num_shares, share_threshold);
    
    std::cout << "\n4. Split Vector into " << shares.size() << " shares (threshold = " << share_threshold << "):" << std::endl;
    for (const auto& [id, share] : shares) {
        std::cout << "   Share " << id << ", Layer 0: ";
        printVector(share.getLayer(0));
        std::cout << std::endl;
    }
    
    // Reconstruct the vector from a subset of shares
    std::unordered_map<size_t, lmvs::LayeredVector> subset_shares;
    size_t count = 0;
    for (const auto& [id, share] : shares) {
        subset_shares[id] = share;
        count++;
        if (count >= share_threshold) {
            break;
        }
    }
    
    lmvs::LayeredVector reconstructed = system.reconstructVector(subset_shares, share_threshold);
    
    std::cout << "\n5. Reconstructed Vector from " << subset_shares.size() << " shares:" << std::endl;
    reconstructed.print();
    
    // Encrypt the vector
    std::vector<std::string> keys = {"key1", "key2", "key3"};
    lmvs::LayeredVector encrypted = system.encryptVector(vector, keys);
    
    std::cout << "\n6. Encrypted Vector:" << std::endl;
    encrypted.print();
    
    // Decrypt the vector
    lmvs::LayeredVector decrypted = system.decryptVector(encrypted, keys);
    
    std::cout << "\n7. Decrypted Vector:" << std::endl;
    decrypted.print();
    
    // Consensus example
    std::cout << "\n8. Consensus Example:" << std::endl;
    
    // Create slightly different vectors for each node
    std::vector<lmvs::LayeredVector> node_vectors;
    for (size_t i = 0; i < num_nodes; ++i) {
        std::vector<std::vector<double>> node_data = data;
        // Add some small random variations
        for (auto& layer : node_data) {
            for (auto& val : layer) {
                val += (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.1;
            }
        }
        node_vectors.push_back(system.createVector(node_data));
    }
    
    // Add contributions from each node
    for (size_t i = 0; i < num_nodes; ++i) {
        std::string node_id = "node" + std::to_string(i + 1);
        bool added = system.addConsensusContribution(node_id, node_vectors[i]);
        std::cout << "   Added contribution from " << node_id << ": " << (added ? "success" : "failed") << std::endl;
    }
    
    // Check if consensus has been reached
    bool has_consensus = system.hasConsensus();
    std::cout << "   Consensus reached: " << (has_consensus ? "yes" : "no") << std::endl;
    
    if (has_consensus) {
        // Get the consensus vector
        lmvs::LayeredVector consensus = system.getConsensusVector();
        std::cout << "   Consensus Vector:" << std::endl;
        consensus.print();
        
        // Validate a vector against the consensus
        bool valid = system.validateVector(vector);
        std::cout << "   Original vector is valid according to consensus: " << (valid ? "yes" : "no") << std::endl;
    }
    
    std::cout << "\nLMVS Example Completed Successfully!" << std::endl;
    
    return 0;
}
