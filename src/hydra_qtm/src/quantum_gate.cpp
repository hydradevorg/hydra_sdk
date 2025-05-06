#include "hydra_qtm/quantum_gate.hpp"
#include <cmath>
#include <complex>

namespace hydra::qtm {

QuantumGate::QuantumGate(const Matrix& matrix, const std::string& label)
    : mat_(matrix), label_(label) {}

const Matrix& QuantumGate::matrix() const { return mat_; }
const std::string& QuantumGate::label() const { return label_; }

// --- Single-qubit gates ---

QuantumGate QuantumGate::I() {
    return QuantumGate(Matrix::Identity(2, 2), "I");
}

QuantumGate QuantumGate::X() {
    Matrix m(2, 2);
    m << 0, 1,
         1, 0;
    return QuantumGate(m, "X");
}

QuantumGate QuantumGate::Y() {
    Matrix m(2, 2);
    m << 0, -std::complex<double>(0, 1),
         std::complex<double>(0, 1), 0;
    return QuantumGate(m, "Y");
}

QuantumGate QuantumGate::Z() {
    Matrix m(2, 2);
    m << 1, 0,
         0, -1;
    return QuantumGate(m, "Z");
}

QuantumGate QuantumGate::H() {
    Matrix m(2, 2);
    m << 1, 1,
         1, -1;
    return QuantumGate(m / std::sqrt(2.0), "H");
}

QuantumGate QuantumGate::S() {
    Matrix m(2, 2);
    m << 1, 0,
         0, std::complex<double>(0, 1);
    return QuantumGate(m, "S");
}

QuantumGate QuantumGate::Sd() {
    Matrix m(2, 2);
    m << 1, 0,
         0, std::complex<double>(0, -1);
    return QuantumGate(m, "S†");
}

QuantumGate QuantumGate::T() {
    Matrix m(2, 2);
    m << 1, 0,
         0, std::polar(1.0, M_PI / 4);
    return QuantumGate(m, "T");
}

QuantumGate QuantumGate::Td() {
    Matrix m(2, 2);
    m << 1, 0,
         0, std::polar(1.0, -M_PI / 4);
    return QuantumGate(m, "T†");
}

QuantumGate QuantumGate::U(double theta, double phi, double lambda) {
    Matrix m(2, 2);
    m << std::cos(theta / 2), -std::exp(std::complex<double>(0, lambda)) * std::sin(theta / 2),
         std::exp(std::complex<double>(0, phi)) * std::sin(theta / 2),
         std::exp(std::complex<double>(0, phi + lambda)) * std::cos(theta / 2);
    return QuantumGate(m, "U");
}

// --- Multi-qubit gates ---

QuantumGate QuantumGate::CNOT() {
    Matrix m = Matrix::Zero(4, 4);
    m(0, 0) = 1;
    m(1, 1) = 1;
    m(2, 3) = 1;
    m(3, 2) = 1;
    return QuantumGate(m, "CNOT");
}

QuantumGate QuantumGate::CZ() {
    Matrix m = Matrix::Identity(4, 4);
    m(3, 3) = -1;
    return QuantumGate(m, "CZ");
}

QuantumGate QuantumGate::SWAP() {
    Matrix m = Matrix::Identity(4, 4);
    m(1, 1) = 0; m(1, 2) = 1;
    m(2, 2) = 0; m(2, 1) = 1;
    return QuantumGate(m, "SWAP");
}

QuantumGate QuantumGate::Toffoli() {
    Matrix m = Matrix::Identity(8, 8);
    m(6, 6) = 0; m(7, 7) = 0;
    m(6, 7) = 1; m(7, 6) = 1;
    return QuantumGate(m, "CCX");
}

} // namespace hydra::qtm
