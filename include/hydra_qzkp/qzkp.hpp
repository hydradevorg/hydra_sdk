#pragma once
#include "hydra_qzkp/quantum_state_vector.hpp"
#include "hydra_qzkp/result_cache.hpp"
#include <string>
#include <vector>
#include <optional>
#include <tuple>
#include <nlohmann/json.hpp>

namespace hydra::qzkp {

class QuantumZKP {
public:
    using Complex = std::complex<double>;
    using Proof = nlohmann::json;

    QuantumZKP(size_t dimensions = 8, size_t security_level = 128);

    std::pair<std::vector<uint8_t>, Proof>
    prove_vector_knowledge(const std::vector<Complex>& vector, const std::string& identifier);

    bool verify_proof(const std::vector<uint8_t>& commitment, const Proof& proof, const std::string& identifier);

    std::vector<bool> verify_proof_batch(
        const std::vector<std::tuple<std::vector<uint8_t>, Proof, std::string>>& batch,
        size_t batch_size);

private:
    std::vector<uint8_t> generate_commitment(const QuantumStateVector& state, const std::string& identifier);
    double calculate_entropy(const std::vector<Complex>& state_coords);
    Proof generate_measurements(const std::vector<Complex>& state_coords);
    bool verify_measurements(const Proof& measurements, size_t state_size);
    bool verify_coefficients(const std::vector<Complex>& coeffs);
    std::vector<uint8_t> prepare_message_for_signing(const Proof& proof, const std::vector<uint8_t>& commitment);
    bool validate_proof_structure(const Proof& proof);

    size_t dimensions_;
    size_t security_level_;

    // Crypto fields
    std::vector<uint8_t> public_key_;
    std::vector<uint8_t> secret_key_;

    ResultCache<std::string, bool> result_cache_;
};

} // namespace hydra::qzkp
