#include "hydra_qtm/simulator.hpp"

namespace hydra::qtm {

Simulator::Simulator(size_t num_qubits, const BackendConfig& config)
    : state_(num_qubits), creg_(num_qubits), config_(config) {}

void Simulator::run(const QuantumCircuit& circuit) {
    for (const auto& [gate, targets] : circuit.operations()) {
        // Future: apply noise here using `noise_` if set
        state_.apply_gate(gate.matrix(), targets);
    }
}

std::unordered_map<std::string, size_t> Simulator::measure_all(size_t shots) {
    std::unordered_map<std::string, size_t> histogram;

    for (size_t i = 0; i < shots; ++i) {
        QuantumState copy = state_;
        std::string outcome = copy.sample_measurement();
        histogram[outcome]++;

        for (size_t j = 0; j < outcome.size(); ++j)
            creg_.set_bit(j, outcome[j] == '1');
    }

    return histogram;
}

void Simulator::set_noise_model(std::shared_ptr<NoiseModel> model) {
    noise_ = model;
}

const QuantumState& Simulator::state() const {
    return state_;
}

const ClassicalRegister& Simulator::classical_bits() const {
    return creg_;
}

const BackendConfig& Simulator::config() const {
    return config_;
}

} // namespace hydra::qtm
