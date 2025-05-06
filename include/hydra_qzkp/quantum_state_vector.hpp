#pragma once

#include <vector>
#include <string>
#include <complex>

#include <unordered_map>
#include <cstdint>

#if __has_include(<optional>)
    #include <optional>
#else
    #include <experimental/optional>
    namespace std { using std::experimental::optional; }
#endif


namespace hydra::qzkp {

class QuantumStateVector {
public:
    using Complex = std::complex<double>;

    explicit QuantumStateVector(std::vector<Complex> coords);

    double entanglement() const;
    void set_entanglement(double e);

    double coherence() const;
    void set_coherence(double c);

    const std::string& state_type() const;
    void set_state_type(const std::string& type);

    double timestamp() const;

    std::vector<uint8_t> serialize() const;
    void clear_cache();

    bool operator==(const QuantumStateVector& other) const;
    std::size_t hash() const;

    const std::vector<Complex>& coordinates() const;

private:
    std::vector<Complex> coordinates_;
    double entanglement_;
    mutable std::optional<double> coherence_;
    std::string state_type_;
    double timestamp_;
    mutable std::unordered_map<std::string, double> cache_;
    mutable std::optional<std::vector<uint8_t>> serialized_;
};

} // namespace hydra::qzkp
