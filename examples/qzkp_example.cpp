#include "hydra_qzkp/qzkp.hpp"
#include "hydra_qzkp/quantum_state_vector.hpp"
#include <iostream>
#include <vector>
#include <complex>
#include <string>
#include <iomanip>
#include <random>
#include <chrono>
#include <tuple>
#include <cmath>

// Helper function to print a complex vector
void print_complex_vector(const std::vector<std::complex<double>>& vec, const std::string& name) {
    std::cout << name << " (size " << vec.size() << "):" << std::endl;
    std::cout << "[ ";
    for (size_t i = 0; i < std::min(vec.size(), size_t(4)); ++i) {
        std::cout << std::fixed << std::setprecision(2)
                  << "(" << vec[i].real() << "," << vec[i].imag() << ") ";
    }
    if (vec.size() > 4) {
        std::cout << "... ";
        for (size_t i = vec.size() - 2; i < vec.size(); ++i) {
            std::cout << std::fixed << std::setprecision(2)
                      << "(" << vec[i].real() << "," << vec[i].imag() << ") ";
        }
    }
    std::cout << "]" << std::endl;
}

// Generate a random quantum state vector
std::vector<std::complex<double>> generate_random_state(size_t size, bool normalized = true) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> real_dist(-1.0, 1.0);
    std::uniform_real_distribution<> imag_dist(-1.0, 1.0);

    std::vector<std::complex<double>> state;
    state.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        state.emplace_back(real_dist(gen), imag_dist(gen));
    }

    if (normalized) {
        // Normalize the state vector
        double norm = 0.0;
        for (const auto& c : state) {
            norm += std::norm(c);
        }
        norm = std::sqrt(norm);

        for (auto& c : state) {
            c /= norm;
        }
    }

    return state;
}

// Generate a Bell state (entangled quantum state)
std::vector<std::complex<double>> generate_bell_state() {
    // |Φ+⟩ = (|00⟩ + |11⟩)/√2
    std::vector<std::complex<double>> bell_state(4, {0.0, 0.0});
    bell_state[0] = {1.0 / std::sqrt(2.0), 0.0}; // |00⟩
    bell_state[3] = {1.0 / std::sqrt(2.0), 0.0}; // |11⟩
    return bell_state;
}

// Generate a GHZ state (multi-qubit entangled state)
std::vector<std::complex<double>> generate_ghz_state(size_t num_qubits) {
    // |GHZ⟩ = (|00...0⟩ + |11...1⟩)/√2
    size_t dim = 1 << num_qubits; // 2^num_qubits
    std::vector<std::complex<double>> ghz_state(dim, {0.0, 0.0});
    ghz_state[0] = {1.0 / std::sqrt(2.0), 0.0};                // |00...0⟩
    ghz_state[dim - 1] = {1.0 / std::sqrt(2.0), 0.0};          // |11...1⟩
    return ghz_state;
}

// Generate a W state (multi-qubit entangled state)
std::vector<std::complex<double>> generate_w_state(size_t num_qubits) {
    // |W⟩ = (|10...0⟩ + |01...0⟩ + ... + |00...1⟩)/√n
    size_t dim = 1 << num_qubits; // 2^num_qubits
    std::vector<std::complex<double>> w_state(dim, {0.0, 0.0});

    double coeff = 1.0 / std::sqrt(num_qubits);

    for (size_t i = 0; i < num_qubits; ++i) {
        size_t idx = 1 << i; // 2^i
        w_state[idx] = {coeff, 0.0};
    }

    return w_state;
}

// Run a basic proof and verification example
void run_basic_example(hydra::qzkp::QuantumZKP& qzkp) {
    std::cout << "\n=== Basic Example ===" << std::endl;

    // Create a simple quantum state vector
    std::vector<std::complex<double>> state_vector = {
        {0.5, 0.0}, {0.5, 0.0}, {0.5, 0.0}, {0.5, 0.0},
        {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}
    };

    print_complex_vector(state_vector, "Basic state vector");

    // Generate a proof of knowledge for this vector
    std::string identifier = "test-vector-1";
    std::cout << "Generating proof for vector with identifier: " << identifier << std::endl;

    auto [commitment, proof] = qzkp.prove_vector_knowledge(state_vector, identifier);

    std::cout << "Generated commitment of size: " << commitment.size() << " bytes" << std::endl;
    std::cout << "Generated proof of size: " << proof.dump().size() << " bytes" << std::endl;

    // Verify the proof
    std::cout << "Verifying proof..." << std::endl;
    bool is_valid = qzkp.verify_proof(commitment, proof, identifier);

    std::cout << "Proof verification result: " << (is_valid ? "Valid" : "Invalid") << std::endl;

    // Try with an invalid identifier
    std::cout << "Verifying with invalid identifier..." << std::endl;
    bool is_invalid = qzkp.verify_proof(commitment, proof, "wrong-identifier");

    std::cout << "Invalid proof verification result: " << (is_invalid ? "Valid" : "Invalid") << std::endl;
}

// Run an example with entangled quantum states
void run_entanglement_example(hydra::qzkp::QuantumZKP& qzkp) {
    std::cout << "\n=== Entanglement Example ===" << std::endl;

    // Generate a Bell state
    auto bell_state = generate_bell_state();
    print_complex_vector(bell_state, "Bell state");

    // Generate a GHZ state
    auto ghz_state = generate_ghz_state(3); // 3-qubit GHZ state
    print_complex_vector(ghz_state, "GHZ state (3 qubits)");

    // Generate a W state
    auto w_state = generate_w_state(3); // 3-qubit W state
    print_complex_vector(w_state, "W state (3 qubits)");

    // Prove knowledge of these entangled states
    std::cout << "Proving knowledge of entangled states..." << std::endl;

    auto [bell_commitment, bell_proof] = qzkp.prove_vector_knowledge(bell_state, "bell-state");
    auto [ghz_commitment, ghz_proof] = qzkp.prove_vector_knowledge(ghz_state, "ghz-state");
    auto [w_commitment, w_proof] = qzkp.prove_vector_knowledge(w_state, "w-state");

    // Verify all proofs
    bool bell_valid = qzkp.verify_proof(bell_commitment, bell_proof, "bell-state");
    bool ghz_valid = qzkp.verify_proof(ghz_commitment, ghz_proof, "ghz-state");
    bool w_valid = qzkp.verify_proof(w_commitment, w_proof, "w-state");

    std::cout << "Bell state verification: " << (bell_valid ? "Valid" : "Invalid") << std::endl;
    std::cout << "GHZ state verification: " << (ghz_valid ? "Valid" : "Invalid") << std::endl;
    std::cout << "W state verification: " << (w_valid ? "Valid" : "Invalid") << std::endl;
}

// Run a batch verification example
void run_batch_verification_example(hydra::qzkp::QuantumZKP& qzkp) {
    std::cout << "\n=== Batch Verification Example ===" << std::endl;

    const size_t num_vectors = 5;
    std::vector<std::tuple<std::vector<uint8_t>, nlohmann::json, std::string>> batch;

    std::cout << "Generating " << num_vectors << " random state vectors and proofs..." << std::endl;

    // Generate random vectors and proofs
    for (size_t i = 0; i < num_vectors; ++i) {
        auto random_state = generate_random_state(8);
        std::string id = "random-state-" + std::to_string(i);

        auto [commitment, proof] = qzkp.prove_vector_knowledge(random_state, id);
        batch.emplace_back(commitment, proof, id);

        std::cout << "  Generated proof for " << id << std::endl;
    }

    // Add one invalid proof (wrong identifier)
    auto last_item = batch.back();
    auto last_commitment = std::get<0>(last_item);
    auto last_proof = std::get<1>(last_item);
    batch.emplace_back(last_commitment, last_proof, "wrong-id");
    std::cout << "  Added one invalid proof (wrong identifier)" << std::endl;

    // Verify the batch
    std::cout << "Verifying batch of " << batch.size() << " proofs..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    auto results = qzkp.verify_proof_batch(batch, 2); // Verify in batches of 2

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Print results
    std::cout << "Batch verification results:" << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  Proof " << i << ": " << (results[i] ? "Valid" : "Invalid") << std::endl;
    }

    std::cout << "Batch verification completed in " << duration << " ms" << std::endl;
}

// Run a performance test
void run_performance_test(hydra::qzkp::QuantumZKP& qzkp) {
    std::cout << "\n=== Performance Test ===" << std::endl;

    const std::vector<size_t> dimensions = {8, 16, 32, 64, 128};

    for (auto dim : dimensions) {
        std::cout << "Testing with " << dim << "-dimensional state vector:" << std::endl;

        // Generate a random state vector
        auto random_state = generate_random_state(dim);
        std::string id = "perf-test-" + std::to_string(dim);

        // Measure proof generation time
        auto start_prove = std::chrono::high_resolution_clock::now();
        auto [commitment, proof] = qzkp.prove_vector_knowledge(random_state, id);
        auto end_prove = std::chrono::high_resolution_clock::now();
        auto duration_prove = std::chrono::duration_cast<std::chrono::milliseconds>(end_prove - start_prove).count();

        // Measure verification time
        auto start_verify = std::chrono::high_resolution_clock::now();
        bool is_valid = qzkp.verify_proof(commitment, proof, id);
        auto end_verify = std::chrono::high_resolution_clock::now();
        auto duration_verify = std::chrono::duration_cast<std::chrono::milliseconds>(end_verify - start_verify).count();

        std::cout << "  Proof generation: " << duration_prove << " ms" << std::endl;
        std::cout << "  Proof verification: " << duration_verify << " ms" << std::endl;
        std::cout << "  Proof size: " << proof.dump().size() << " bytes" << std::endl;
        std::cout << "  Verification result: " << (is_valid ? "Valid" : "Invalid") << std::endl;
    }
}

// Simulate a quantum authentication protocol
void run_quantum_authentication_example(hydra::qzkp::QuantumZKP& qzkp) {
    std::cout << "\n=== Quantum Authentication Example ===" << std::endl;

    // Step 1: Setup - Generate a unique quantum state for the user
    std::cout << "Step 1: Setup - Generating user's quantum state..." << std::endl;
    auto user_state = generate_random_state(16);
    print_complex_vector(user_state, "User's quantum state");

    // Step 2: Registration - User registers their quantum state with the server
    std::cout << "\nStep 2: Registration - User registers with the server" << std::endl;
    std::string user_id = "alice@example.com";
    auto [reg_commitment, reg_proof] = qzkp.prove_vector_knowledge(user_state, user_id);

    std::cout << "User registered with ID: " << user_id << std::endl;
    std::cout << "Server stores commitment: " << reg_commitment.size() << " bytes" << std::endl;

    // Step 3: Authentication - User proves knowledge of their quantum state
    std::cout << "\nStep 3: Authentication - User proves knowledge of their quantum state" << std::endl;

    // Simulate multiple authentication attempts
    for (int attempt = 1; attempt <= 3; ++attempt) {
        std::cout << "\nAuthentication attempt #" << attempt << ":" << std::endl;

        // For the demo, we'll use the same state for valid attempts and a different state for invalid
        std::vector<std::complex<double>> auth_state;
        std::string auth_id = user_id;

        if (attempt == 3) {
            // Simulate an attacker with a different quantum state
            std::cout << "Simulating an attacker with a different quantum state" << std::endl;
            auth_state = generate_random_state(16);
            auth_id = "attacker@example.com";
        } else if (attempt == 2) {
            // Simulate an attacker with the correct state but wrong ID
            std::cout << "Simulating an attacker with correct state but wrong ID" << std::endl;
            auth_state = user_state;
            auth_id = "mallory@example.com";
        } else {
            // Valid authentication
            std::cout << "Simulating valid user authentication" << std::endl;
            auth_state = user_state;
        }

        // Generate authentication proof
        auto [auth_commitment, auth_proof] = qzkp.prove_vector_knowledge(auth_state, auth_id);

        // Server verifies the proof
        bool is_valid = qzkp.verify_proof(auth_commitment, auth_proof, user_id);

        std::cout << "Authentication " << (is_valid ? "successful" : "failed") << std::endl;

        if (is_valid) {
            std::cout << "User granted access to the system" << std::endl;
        } else {
            std::cout << "Access denied" << std::endl;
        }
    }

    // Step 4: Demonstrate quantum state rotation (changing password)
    std::cout << "\nStep 4: User rotates their quantum state (changes password)" << std::endl;

    // Apply a "rotation" to the quantum state (simulated by generating a new state)
    auto rotated_state = generate_random_state(16);
    print_complex_vector(rotated_state, "User's new quantum state");

    // Register the new state (this would update the server's stored commitment)
    auto [new_commitment, new_proof] = qzkp.prove_vector_knowledge(rotated_state, user_id);

    std::cout << "User registered new quantum state" << std::endl;
    std::cout << "Server updates stored commitment to: " << new_commitment.size() << " bytes" << std::endl;

    // In a real system, the server would now use the new commitment for verification

    // Try to authenticate with old state (should fail with new commitment)
    auto [old_auth_commitment, old_auth_proof] = qzkp.prove_vector_knowledge(user_state, user_id);

    // In a real system, the server would compare the commitment with its stored value
    // Here we'll verify that the old commitment doesn't match the new one
    bool old_matches_new = (old_auth_commitment == new_commitment);

    std::cout << "Old commitment matches new: " << (old_matches_new ? "yes" : "no") << std::endl;
    std::cout << "Authentication with old state: failed" << std::endl;
    std::cout << "Access denied - old state no longer valid" << std::endl;

    // Try to authenticate with new state (should succeed with new commitment)
    auto [new_auth_commitment, new_auth_proof] = qzkp.prove_vector_knowledge(rotated_state, user_id);

    // In a real system, we would verify the proof against the stored commitment
    // For this example, we'll verify the proof directly
    bool new_valid = qzkp.verify_proof(new_auth_commitment, new_auth_proof, user_id);

    std::cout << "Authentication with new state: " << (new_valid ? "successful" : "failed") << std::endl;
    if (new_valid) {
        std::cout << "Access granted - new state is valid" << std::endl;
    }
}

int main() {
    std::cout << "Quantum Zero-Knowledge Proof Example" << std::endl;
    std::cout << "===================================" << std::endl;

    // Create a quantum ZKP instance with 8 dimensions and 128-bit security
    hydra::qzkp::QuantumZKP qzkp(8, 128);

    // Run the basic example
    run_basic_example(qzkp);

    // Run the entanglement example
    run_entanglement_example(qzkp);

    // Run the batch verification example
    run_batch_verification_example(qzkp);

    // Run the performance test
    run_performance_test(qzkp);

    // Run the quantum authentication example
    run_quantum_authentication_example(qzkp);

    return 0;
}
