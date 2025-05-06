#include "lmvs/lmvs.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>

// Helper function to check if two vectors are approximately equal
bool approxEqual(const std::vector<double>& a, const std::vector<double>& b, double epsilon = 1e-6) {
    if (a.size() != b.size()) {
        return false;
    }
    
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::abs(a[i] - b[i]) > epsilon) {
            return false;
        }
    }
    
    return true;
}

// Helper function to check if two layered vectors are approximately equal
bool approxEqual(const lmvs::LayeredVector& a, const lmvs::LayeredVector& b, double epsilon = 1e-6) {
    if (a.getNumLayers() != b.getNumLayers() || a.getDimension() != b.getDimension()) {
        return false;
    }
    
    for (size_t i = 0; i < a.getNumLayers(); ++i) {
        if (!approxEqual(a.getLayer(i), b.getLayer(i), epsilon)) {
            return false;
        }
    }
    
    return true;
}

// Test LayeredVector
void testLayeredVector() {
    std::cout << "Testing LayeredVector..." << std::endl;
    
    // Create a layered vector
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},
        {5.0, 6.0, 7.0, 8.0},
        {9.0, 10.0, 11.0, 12.0}
    };
    
    lmvs::LayeredVector vector(data);
    
    // Check dimensions
    assert(vector.getNumLayers() == 3);
    assert(vector.getDimension() == 4);
    
    // Check layer access
    assert(approxEqual(vector.getLayer(0), {1.0, 2.0, 3.0, 4.0}));
    assert(approxEqual(vector.getLayer(1), {5.0, 6.0, 7.0, 8.0}));
    assert(approxEqual(vector.getLayer(2), {9.0, 10.0, 11.0, 12.0}));
    
    // Test layer modification
    std::vector<double> new_layer = {13.0, 14.0, 15.0, 16.0};
    vector.setLayer(1, new_layer);
    assert(approxEqual(vector.getLayer(1), new_layer));
    
    // Test adding a layer
    std::vector<double> added_layer = {17.0, 18.0, 19.0, 20.0};
    vector.addLayer(added_layer);
    assert(vector.getNumLayers() == 4);
    assert(approxEqual(vector.getLayer(3), added_layer));
    
    // Test removing a layer
    vector.removeLayer(3);
    assert(vector.getNumLayers() == 3);
    
    // Test squared distance
    lmvs::LayeredVector vector2(data);
    assert(vector.squaredDistance(vector2) > 0.0); // Should be different due to the modified layer
    
    std::cout << "LayeredVector tests passed!" << std::endl;
}

// Test LayeredMatrix
void testLayeredMatrix() {
    std::cout << "Testing LayeredMatrix..." << std::endl;
    
    // Create two layered vectors
    std::vector<std::vector<double>> data1 = {
        {1.0, 2.0},
        {3.0, 4.0}
    };
    
    std::vector<std::vector<double>> data2 = {
        {0.1, 0.2},
        {0.3, 0.4}
    };
    
    lmvs::LayeredVector vec1(data1);
    lmvs::LayeredVector vec2(data2);
    
    // Create a layered matrix from the two vectors
    lmvs::LayeredMatrix matrix(vec1, vec2);
    
    // Check dimensions
    assert(matrix.getNumLayers() == 2);
    assert(matrix.getRowDimension() == 2);
    assert(matrix.getColDimension() == 2);
    
    // Check block access
    const auto& block00 = matrix.getBlock(0, 0);
    assert(std::abs(block00[0][0] - 0.1) < 1e-6);
    assert(std::abs(block00[0][1] - 0.2) < 1e-6);
    assert(std::abs(block00[1][0] - 0.2) < 1e-6);
    assert(std::abs(block00[1][1] - 0.4) < 1e-6);
    
    // Test block modification
    std::vector<std::vector<double>> new_block = {
        {1.1, 1.2},
        {1.3, 1.4}
    };
    matrix.setBlock(0, 1, new_block);
    assert(approxEqual(matrix.getBlock(0, 1)[0], new_block[0]));
    assert(approxEqual(matrix.getBlock(0, 1)[1], new_block[1]));
    
    // Test outer product
    std::vector<double> vec_a = {2.0, 3.0};
    std::vector<double> vec_b = {4.0, 5.0};
    matrix.setBlockFromOuterProduct(1, 0, vec_a, vec_b);
    
    const auto& block10 = matrix.getBlock(1, 0);
    assert(std::abs(block10[0][0] - 8.0) < 1e-6);
    assert(std::abs(block10[0][1] - 10.0) < 1e-6);
    assert(std::abs(block10[1][0] - 12.0) < 1e-6);
    assert(std::abs(block10[1][1] - 15.0) < 1e-6);
    
    std::cout << "LayeredMatrix tests passed!" << std::endl;
}

// Test ProjectionMatrix
void testProjectionMatrix() {
    std::cout << "Testing ProjectionMatrix..." << std::endl;
    
    // Create a projection matrix
    const size_t input_dim = 4;
    const size_t output_dim = 2;
    lmvs::ProjectionMatrix proj(input_dim, output_dim);
    
    // Check dimensions
    assert(proj.getInputDimension() == input_dim);
    assert(proj.getOutputDimension() == output_dim);
    
    // Test vector projection
    std::vector<double> vec = {1.0, 2.0, 3.0, 4.0};
    std::vector<double> projected = proj.project(vec);
    
    assert(projected.size() == output_dim);
    assert(std::abs(projected[0] - 1.0) < 1e-6); // Identity projection for first two dimensions
    assert(std::abs(projected[1] - 2.0) < 1e-6);
    
    // Test layered vector projection
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0, 4.0},
        {5.0, 6.0, 7.0, 8.0}
    };
    
    lmvs::LayeredVector layered_vec(data);
    lmvs::LayeredVector projected_vec = proj.project(layered_vec);
    
    assert(projected_vec.getNumLayers() == layered_vec.getNumLayers());
    assert(projected_vec.getDimension() == output_dim);
    assert(std::abs(projected_vec.getLayer(0)[0] - 1.0) < 1e-6);
    assert(std::abs(projected_vec.getLayer(0)[1] - 2.0) < 1e-6);
    assert(std::abs(projected_vec.getLayer(1)[0] - 5.0) < 1e-6);
    assert(std::abs(projected_vec.getLayer(1)[1] - 6.0) < 1e-6);
    
    // Test random projection
    lmvs::ProjectionMatrix random_proj = lmvs::ProjectionMatrix::createRandom(input_dim, output_dim, 42);
    std::vector<double> random_projected = random_proj.project(vec);
    assert(random_projected.size() == output_dim);
    
    // Test orthogonal projection
    lmvs::ProjectionMatrix ortho_proj = lmvs::ProjectionMatrix::createOrthogonal(input_dim, output_dim, 42);
    std::vector<double> ortho_projected = ortho_proj.project(vec);
    assert(ortho_projected.size() == output_dim);
    
    std::cout << "ProjectionMatrix tests passed!" << std::endl;
}

// Test SecureConsensusLayer
void testSecureConsensusLayer() {
    std::cout << "Testing SecureConsensusLayer..." << std::endl;
    
    // Create a consensus layer
    const size_t num_nodes = 5;
    const size_t threshold = 3;
    lmvs::SecureConsensusLayer consensus(num_nodes, threshold);
    
    // Create a base vector
    std::vector<std::vector<double>> base_data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    lmvs::LayeredVector base_vector(base_data);
    
    // Create slightly different vectors for each node
    std::vector<lmvs::LayeredVector> node_vectors;
    for (size_t i = 0; i < num_nodes; ++i) {
        std::vector<std::vector<double>> node_data = base_data;
        // Add some small random variations
        for (auto& layer : node_data) {
            for (auto& val : layer) {
                val += (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.01;
            }
        }
        node_vectors.push_back(lmvs::LayeredVector(node_data));
    }
    
    // Add contributions from each node
    for (size_t i = 0; i < num_nodes; ++i) {
        std::string node_id = "node" + std::to_string(i + 1);
        bool added = consensus.addContribution(node_id, node_vectors[i]);
        assert(added);
    }
    
    // Check if consensus has been reached
    assert(consensus.hasConsensus());
    
    // Get the consensus vector
    lmvs::LayeredVector consensus_vector = consensus.getConsensusVector();
    
    // Validate the base vector against the consensus
    bool valid = consensus.validateVector(base_vector);
    assert(valid);
    
    // Create a very different vector
    std::vector<std::vector<double>> different_data = {
        {10.0, 20.0, 30.0},
        {40.0, 50.0, 60.0}
    };
    
    lmvs::LayeredVector different_vector(different_data);
    
    // This should not be valid according to the consensus
    valid = consensus.validateVector(different_vector);
    assert(!valid);
    
    // Test reset
    consensus.reset();
    assert(consensus.getNumContributions() == 0);
    assert(!consensus.hasConsensus());
    
    std::cout << "SecureConsensusLayer tests passed!" << std::endl;
}

// Test LayeredSecretSharing
void testLayeredSecretSharing() {
    std::cout << "Testing LayeredSecretSharing..." << std::endl;
    
    // Create a secret sharing instance
    const size_t threshold = 3;
    const size_t num_shares = 5;
    lmvs::LayeredSecretSharing secret_sharing(threshold, num_shares);
    
    // Create a secret vector
    std::vector<std::vector<double>> secret_data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    lmvs::LayeredVector secret(secret_data);
    
    // Split the secret into shares
    auto shares = secret_sharing.split(secret);
    
    // Check that we have the correct number of shares
    assert(shares.size() == num_shares);
    
    // Reconstruct from all shares
    lmvs::LayeredVector reconstructed = secret_sharing.reconstruct(shares);
    
    // Check that the reconstructed secret matches the original
    assert(approxEqual(reconstructed, secret, 1e-6));
    
    // Reconstruct from a subset of shares
    std::unordered_map<size_t, lmvs::LayeredVector> subset_shares;
    size_t count = 0;
    for (const auto& [id, share] : shares) {
        subset_shares[id] = share;
        count++;
        if (count >= threshold) {
            break;
        }
    }
    
    lmvs::LayeredVector reconstructed_subset = secret_sharing.reconstruct(subset_shares);
    
    // Check that the reconstructed secret matches the original
    assert(approxEqual(reconstructed_subset, secret, 1e-6));
    
    // Try with fewer shares than the threshold (should throw)
    std::unordered_map<size_t, lmvs::LayeredVector> too_few_shares;
    count = 0;
    for (const auto& [id, share] : shares) {
        too_few_shares[id] = share;
        count++;
        if (count >= threshold - 1) {
            break;
        }
    }
    
    bool exception_thrown = false;
    try {
        secret_sharing.reconstruct(too_few_shares);
    } catch (const std::invalid_argument&) {
        exception_thrown = true;
    }
    
    assert(exception_thrown);
    
    std::cout << "LayeredSecretSharing tests passed!" << std::endl;
}

// Test LayerEncryption
void testLayerEncryption() {
    std::cout << "Testing LayerEncryption..." << std::endl;
    
    // Create an encryption provider
    auto provider = std::make_shared<lmvs::SimpleXOREncryptionProvider>();
    
    // Create a layer encryption instance
    lmvs::LayerEncryption encryption(provider);
    
    // Create a vector to encrypt
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    lmvs::LayeredVector vector(data);
    
    // Encrypt the vector
    std::vector<std::string> keys = {"key1", "key2"};
    lmvs::LayeredVector encrypted = encryption.encrypt(vector, keys);
    
    // Check that the encrypted vector is different from the original
    assert(!approxEqual(encrypted, vector));
    
    // Decrypt the vector
    lmvs::LayeredVector decrypted = encryption.decrypt(encrypted, keys);
    
    // Check that the decrypted vector matches the original
    assert(approxEqual(decrypted, vector));
    
    // Try with wrong keys
    std::vector<std::string> wrong_keys = {"wrong1", "wrong2"};
    lmvs::LayeredVector wrong_decrypted = encryption.decrypt(encrypted, wrong_keys);
    
    // Check that the wrong decryption doesn't match the original
    assert(!approxEqual(wrong_decrypted, vector));
    
    std::cout << "LayerEncryption tests passed!" << std::endl;
}

// Test LMVS
void testLMVS() {
    std::cout << "Testing LMVS..." << std::endl;
    
    // Create an LMVS instance
    const size_t num_layers = 2;
    const size_t dimension = 3;
    const size_t num_nodes = 5;
    const size_t threshold = 3;
    
    lmvs::LMVS system(num_layers, dimension, num_nodes, threshold);
    
    // Create a vector
    std::vector<std::vector<double>> data = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0}
    };
    
    lmvs::LayeredVector vector = system.createVector(data);
    
    // Test projection
    const size_t output_dim = 2;
    lmvs::LayeredVector projected = system.projectVector(vector, output_dim);
    
    assert(projected.getNumLayers() == num_layers);
    assert(projected.getDimension() == output_dim);
    
    // Test secret sharing
    const size_t num_shares = 5;
    const size_t share_threshold = 3;
    auto shares = system.splitVector(vector, num_shares, share_threshold);
    
    assert(shares.size() == num_shares);
    
    // Test reconstruction
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
    
    assert(approxEqual(reconstructed, vector, 1e-6));
    
    // Test encryption
    std::vector<std::string> keys = {"key1", "key2"};
    lmvs::LayeredVector encrypted = system.encryptVector(vector, keys);
    
    assert(!approxEqual(encrypted, vector));
    
    // Test decryption
    lmvs::LayeredVector decrypted = system.decryptVector(encrypted, keys);
    
    assert(approxEqual(decrypted, vector));
    
    // Test consensus
    for (size_t i = 0; i < num_nodes; ++i) {
        std::string node_id = "node" + std::to_string(i + 1);
        system.addConsensusContribution(node_id, vector);
    }
    
    assert(system.hasConsensus());
    
    lmvs::LayeredVector consensus = system.getConsensusVector();
    
    assert(approxEqual(consensus, vector));
    
    assert(system.validateVector(vector));
    
    std::cout << "LMVS tests passed!" << std::endl;
}

int main() {
    std::cout << "Running LMVS tests..." << std::endl;
    
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // Run tests
    testLayeredVector();
    testLayeredMatrix();
    testProjectionMatrix();
    testSecureConsensusLayer();
    testLayeredSecretSharing();
    testLayerEncryption();
    testLMVS();
    
    std::cout << "All tests passed!" << std::endl;
    
    return 0;
}
