#pragma once
#include <complex>
#include <vector>
#include <string>
#include <Eigen/Dense>

namespace hydra::qtm {

using Complex = std::complex<double>;

class QuantumState {
public:
    explicit QuantumState(size_t num_qubits);

    void set_basis_state(const std::string& label);
    void normalize();
    std::vector<std::string> basis_labels() const;

    const std::vector<Complex>& amplitudes() const;
    size_t qubit_count() const;

    void apply_gate(const Eigen::MatrixXcd& gate, const std::vector<size_t>& targets);
    std::string sample_measurement();

private:
    size_t n_;
    std::vector<Complex> state_;
};

} // namespace hydra::qtm
