#include "hydra_qtm/quantum_state.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/KroneckerProduct>
#include <cmath>
#include <random>
#include <bitset>
#include <cassert>

namespace hydra::qtm {

QuantumState::QuantumState(size_t num_qubits)
    : n_(num_qubits), state_(1ULL << num_qubits, Complex(0.0, 0.0)) {
    state_[0] = Complex(1.0, 0.0); // Start in |0⟩^n
}

void QuantumState::set_basis_state(const std::string& label) {
    assert(label.size() == n_);
    state_.assign(state_.size(), Complex(0.0));
    size_t index = std::stoull(label, nullptr, 2);
    state_[index] = Complex(1.0, 0.0);
}

void QuantumState::normalize() {
    double norm = 0.0;
    for (const auto& amp : state_) norm += std::norm(amp);
    norm = std::sqrt(norm);
    for (auto& amp : state_) amp /= norm;
}

std::vector<std::string> QuantumState::basis_labels() const {
    std::vector<std::string> labels;
    for (size_t i = 0; i < state_.size(); ++i) {
        labels.emplace_back(std::bitset<64>(i).to_string().substr(64 - n_));
    }
    return labels;
}

const std::vector<Complex>& QuantumState::amplitudes() const {
    return state_;
}

size_t QuantumState::qubit_count() const {
    return n_;
}

void QuantumState::apply_gate(const Eigen::MatrixXcd& gate, const std::vector<size_t>& targets) {
    assert(!targets.empty());

    Eigen::VectorXcd vec(state_.size());
    for (size_t i = 0; i < state_.size(); ++i)
        vec(i) = state_[i];

    Eigen::MatrixXcd full_gate = Eigen::MatrixXcd::Identity(1, 1);
    size_t next_q = 0;

    for (size_t q = 0; q < n_; ++q) {
        Eigen::MatrixXcd g = Eigen::MatrixXcd::Identity(2, 2);
        if (next_q < targets.size() && targets[next_q] == q) {
            g = gate;
            ++next_q;
        }
        full_gate = Eigen::kroneckerProduct(full_gate, g).eval();
    }

    vec = full_gate * vec;
    for (size_t i = 0; i < state_.size(); ++i)
        state_[i] = vec(i);
}

std::string QuantumState::sample_measurement() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist;
    std::vector<double> probs;

    for (const auto& amp : state_)
        probs.push_back(std::norm(amp));

    dist = std::discrete_distribution<>(probs.begin(), probs.end());
    size_t result = dist(gen);

    std::string label = std::bitset<64>(result).to_string().substr(64 - n_);

    // Collapse state
    state_.assign(state_.size(), Complex(0));
    state_[result] = Complex(1);
    return label;
}

} // namespace hydra::qtm
