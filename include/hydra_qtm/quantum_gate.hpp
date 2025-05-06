#pragma once
#include <Eigen/Dense>
#include <string>

namespace hydra::qtm {

using Matrix = Eigen::MatrixXcd;

class QuantumGate {
public:
    QuantumGate(const Matrix& matrix, const std::string& label);

    const Matrix& matrix() const;
    const std::string& label() const;

    // Static generators
    static QuantumGate I();
    static QuantumGate X();
    static QuantumGate Y();
    static QuantumGate Z();
    static QuantumGate H();
    static QuantumGate S();
    static QuantumGate Sd();
    static QuantumGate T();
    static QuantumGate Td();
    static QuantumGate U(double theta, double phi, double lambda);
    static QuantumGate CNOT();
    static QuantumGate CZ();
    static QuantumGate SWAP();
    static QuantumGate Toffoli();

private:
    std::string label_;
    Matrix mat_;
};

} // namespace hydra::qtm
