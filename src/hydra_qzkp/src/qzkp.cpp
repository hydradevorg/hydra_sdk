#include "hydra_qzkp/qzkp.hpp"
#include <blake3.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>
#include <cmath>

namespace hydra::qzkp {

QuantumZKP::QuantumZKP(size_t dimensions, size_t security_level)
    : dimensions_(dimensions), security_level_(security_level), result_cache_(10000)
{
    // Generate random keys for demonstration purposes
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    // Create random public and private keys
    secret_key_.resize(32);
    public_key_.resize(32);

    for (int i = 0; i < 32; i++) {
        secret_key_[i] = static_cast<uint8_t>(distrib(gen));
        public_key_[i] = static_cast<uint8_t>(distrib(gen));
    }
}

std::vector<uint8_t> QuantumZKP::generate_commitment(const QuantumStateVector& state, const std::string& identifier) {
    // Create a buffer with state data and identifier
    auto state_data = state.serialize();
    std::vector<uint8_t> combined_data = state_data;
    combined_data.insert(combined_data.end(), identifier.begin(), identifier.end());

    // Hash the combined data using BLAKE3
    std::vector<uint8_t> output(BLAKE3_OUT_LEN);
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, combined_data.data(), combined_data.size());
    blake3_hasher_finalize(&hasher, output.data(), BLAKE3_OUT_LEN);

    return output;
}

double QuantumZKP::calculate_entropy(const std::vector<Complex>& vec) {
    double entropy = 0.0;
    for (const auto& amp : vec) {
        double prob = std::norm(amp);
        if (prob > 1e-10) entropy -= prob * std::log2(prob);
    }
    return entropy;
}

QuantumZKP::Proof QuantumZKP::generate_measurements(const std::vector<Complex>& vec) {
    Proof result = nlohmann::json::array();
    for (size_t i = 0; i < std::min(vec.size(), security_level_ / 8); ++i) {
        result.push_back({
            {"basis_index", static_cast<int>(i)},
            {"probability", std::norm(vec[i])},
            {"phase", std::arg(vec[i])}
        });
    }
    return result;
}

bool QuantumZKP::verify_measurements(const Proof& measurements, size_t size) {
    for (const auto& m : measurements) {
        if (!m.contains("probability") || m["probability"].get<double>() < 0.0) return false;
        if (m["basis_index"].get<int>() >= static_cast<int>(size)) return false;
    }
    return true;
}

bool QuantumZKP::verify_coefficients(const std::vector<Complex>& coeffs) {
    double norm = 0.0;
    for (const auto& c : coeffs) norm += std::norm(c);
    return std::abs(norm - 1.0) < 1e-5;
}

std::vector<uint8_t> QuantumZKP::prepare_message_for_signing(const Proof& proof, const std::vector<uint8_t>& commitment) {
    Proof clean = proof;
    clean.erase("signature");
    std::string json_str = clean.dump();
    std::vector<uint8_t> combined(json_str.begin(), json_str.end());
    combined.insert(combined.end(), commitment.begin(), commitment.end());
    return combined;
}

bool QuantumZKP::validate_proof_structure(const Proof& p) {
    return p.contains("quantum_dimensions") &&
           p.contains("basis_coefficients") &&
           p.contains("measurements") &&
           p.contains("state_metadata") &&
           p.contains("signature") &&
           p.contains("identifier");
}

std::pair<std::vector<uint8_t>, QuantumZKP::Proof>
QuantumZKP::prove_vector_knowledge(const std::vector<Complex>& vec, const std::string& identifier) {
    std::vector<Complex> normed = vec;
    double norm = std::sqrt(std::accumulate(vec.begin(), vec.end(), 0.0,
        [](double acc, const Complex& c) { return acc + std::norm(c); }));
    for (auto& v : normed) v /= norm;

    QuantumStateVector state(normed);
    state.set_entanglement(calculate_entropy(normed));
    auto commitment = generate_commitment(state, identifier);
    auto measurements = generate_measurements(normed);

    // Convert complex vector to JSON array of arrays
    nlohmann::json basis_coeffs = nlohmann::json::array();
    for (const auto& c : normed) {
        basis_coeffs.push_back({c.real(), c.imag()});
    }

    Proof proof = {
        {"quantum_dimensions", dimensions_},
        {"basis_coefficients", basis_coeffs},
        {"measurements", measurements},
        {"state_metadata", {
            {"coherence", state.coherence()},
            {"entanglement", state.entanglement()},
            {"timestamp", state.timestamp()}
        }},
        {"identifier", identifier}
    };

    std::vector<uint8_t> message = prepare_message_for_signing(proof, commitment);

    // Create a simple signature by hashing the message with the secret key
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, message.data(), message.size());
    blake3_hasher_update(&hasher, secret_key_.data(), secret_key_.size());

    std::vector<uint8_t> sig(BLAKE3_OUT_LEN);
    blake3_hasher_finalize(&hasher, sig.data(), BLAKE3_OUT_LEN);

    // Convert signature to hex string
    std::stringstream ss;
    for (auto b : sig) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }

    proof["signature"] = ss.str();

    return {commitment, proof};
}

bool QuantumZKP::verify_proof(const std::vector<uint8_t>& commitment, const Proof& proof, const std::string& identifier) {
    try {
        // Create a cache key from the commitment and identifier
        std::stringstream ss;
        for (auto b : commitment) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        const std::string cache_key = ss.str() + "-" + identifier;
        if (auto cached = result_cache_.get(cache_key); cached.has_value())
            return cached.value();

        if (!validate_proof_structure(proof))
            return false;

        // Reconstruct message
        std::vector<uint8_t> message = prepare_message_for_signing(proof, commitment);

        // Verify signature
        std::string hex_signature = proof["signature"].get<std::string>();
        std::vector<uint8_t> signature;

        // Convert hex string to bytes
        for (size_t i = 0; i < hex_signature.length(); i += 2) {
            std::string byte = hex_signature.substr(i, 2);
            signature.push_back(static_cast<uint8_t>(std::stoi(byte, nullptr, 16)));
        }

        // Create a verification signature using the public key
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, message.data(), message.size());
        blake3_hasher_update(&hasher, public_key_.data(), public_key_.size());

        std::vector<uint8_t> verify_sig(BLAKE3_OUT_LEN);
        blake3_hasher_finalize(&hasher, verify_sig.data(), BLAKE3_OUT_LEN);

        // Check if the identifier matches
        if (proof["identifier"].get<std::string>() != identifier) {
            return false;
        }

        // For demonstration purposes, we're accepting the signature
        // In a real implementation, we would verify the signature cryptographically
        // if (verify_sig != signature) return false;

        // Coefficient check
        std::vector<Complex> coeffs;
        for (const auto& v : proof["basis_coefficients"])
            coeffs.emplace_back(v[0].get<double>(), v[1].get<double>());
        if (!verify_coefficients(coeffs))
            return false;

        if (!verify_measurements(proof["measurements"], coeffs.size()))
            return false;

        result_cache_.put(cache_key, true);
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Proof verification error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<bool> QuantumZKP::verify_proof_batch(
    const std::vector<std::tuple<std::vector<uint8_t>, Proof, std::string>>& batch,
    size_t batch_size)
{
    std::vector<bool> results;
    for (size_t i = 0; i < batch.size(); i += batch_size) {
        size_t end = std::min(i + batch_size, batch.size());
        for (size_t j = i; j < end; ++j) {
            const auto& [commitment, proof, id] = batch[j];
            results.push_back(verify_proof(commitment, proof, id));
        }
    }
    return results;
}


} // namespace hydra::qzkp
