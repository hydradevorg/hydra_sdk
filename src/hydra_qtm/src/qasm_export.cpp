#include "hydra_qtm/qasm_export.hpp"
#include <sstream>

namespace hydra::qtm {

std::string export_to_json(const QuantumCircuit& qc) {
    std::ostringstream out;
    out << "{ \"qubits\": " << qc.qubit_count() << ", \"gates\": [";
    const auto& ops = qc.operations();

    for (size_t i = 0; i < ops.size(); ++i) {
        const auto& [gate, targets] = ops[i];
        out << "{ \"name\": \"" << gate.label() << "\", \"targets\": [";
        for (size_t j = 0; j < targets.size(); ++j) {
            out << targets[j];
            if (j + 1 < targets.size()) out << ", ";
        }
        out << "] }";
        if (i + 1 < ops.size()) out << ", ";
    }

    out << "] }";
    return out.str();
}

std::string histogram_to_json(const std::unordered_map<std::string, size_t>& hist) {
    std::ostringstream out;
    out << "{ \"results\": {";
    size_t count = 0;
    for (const auto& [key, val] : hist) {
        out << "\"" << key << "\": " << val;
        if (++count < hist.size()) out << ", ";
    }
    out << "} }";
    return out.str();
}

} // namespace hydra::qtm
