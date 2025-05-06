#pragma once
#include "quantum_gate.hpp"
#include <vector>
#include <tuple>

namespace hydra::qtm {

class QuantumCircuit {
public:
    explicit QuantumCircuit(size_t num_qubits);

    void add_gate(const QuantumGate& gate, const std::vector<size_t>& targets);
    size_t qubit_count() const;

    const std::vector<std::tuple<QuantumGate, std::vector<size_t>>>& operations() const;

private:
    size_t n_;
    std::vector<std::tuple<QuantumGate, std::vector<size_t>>> ops_;
};

} // namespace hydra::qtm
