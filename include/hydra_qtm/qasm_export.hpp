#pragma once
#include "quantum_circuit.hpp"
#include <string>
#include <unordered_map>

namespace hydra::qtm {

std::string export_to_json(const QuantumCircuit& qc);
std::string histogram_to_json(const std::unordered_map<std::string, size_t>& hist);

} // namespace hydra::qtm
