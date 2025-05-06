#pragma once
#include "quantum_state.hpp"
#include "quantum_circuit.hpp"
#include "classical_register.hpp"
#include "backend.hpp"
#include "noise_model.hpp"
#include <unordered_map>
#include <memory>

namespace hydra::qtm {

class Simulator {
public:
    explicit Simulator(size_t num_qubits, const BackendConfig& config = {});

    void run(const QuantumCircuit& circuit);
    std::unordered_map<std::string, size_t> measure_all(size_t shots = 1024);

    void set_noise_model(std::shared_ptr<NoiseModel> model);
    const QuantumState& state() const;
    const ClassicalRegister& classical_bits() const;
    const BackendConfig& config() const;

private:
    QuantumState state_;
    ClassicalRegister creg_;
    BackendConfig config_;
    std::shared_ptr<NoiseModel> noise_;
};

} // namespace hydra::qtm
