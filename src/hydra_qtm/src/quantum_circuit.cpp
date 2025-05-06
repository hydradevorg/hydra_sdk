#include "hydra_qtm/quantum_circuit.hpp"

namespace hydra::qtm {

QuantumCircuit::QuantumCircuit(size_t num_qubits)
    : n_(num_qubits) {}

void QuantumCircuit::add_gate(const QuantumGate& gate, const std::vector<size_t>& targets) {
    ops_.emplace_back(gate, targets);
}

size_t QuantumCircuit::qubit_count() const {
    return n_;
}

const std::vector<std::tuple<QuantumGate, std::vector<size_t>>>& QuantumCircuit::operations() const {
    return ops_;
}

} // namespace hydra::qtm
