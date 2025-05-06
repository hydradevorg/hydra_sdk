#include "hydra_qzkp/quantum_state_vector.hpp"
#include <numeric>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

namespace hydra::qzkp {

using json = nlohmann::json;

QuantumStateVector::QuantumStateVector(std::vector<Complex> coords)
    : coordinates_(std::move(coords)),
      entanglement_(0.0),
      coherence_(std::nullopt),
      state_type_("SUPERPOSITION"),
      timestamp_(std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count())
{
    if (coordinates_.size() > 1024) {
        // Could log a warning if needed
    }
}

double QuantumStateVector::entanglement() const {
    return entanglement_;
}

void QuantumStateVector::set_entanglement(double e) {
    entanglement_ = e;
}

double QuantumStateVector::coherence() {
    if (!coherence_) {
        double sum = std::accumulate(coordinates_.begin(), coordinates_.end(), 0.0,
            [](double acc, const Complex& c) { return acc + std::abs(c); });
        *coherence_ = sum / coordinates_.size();
        cache_["coherence"] = *coherence_;
    }
    return *coherence_;
}

void QuantumStateVector::set_coherence(double c) {
    coherence_ = c;
    cache_["coherence"] = c;
}

const std::string& QuantumStateVector::state_type() const {
    return state_type_;
}

void QuantumStateVector::set_state_type(const std::string& type) {
    state_type_ = type;
}

double QuantumStateVector::timestamp() const {
    return timestamp_;
}

std::vector<uint8_t> QuantumStateVector::serialize() const {
    if (serialized_) return *serialized_;

    json j;
    j["coordinates"] = coordinates_;
    j["entanglement"] = entanglement_;
    j["coherence"] = coherence_.value_or(this->coherence());
    j["state_type"] = state_type_;
    j["timestamp"] = timestamp_;

    std::string str = j.dump();
    std::vector<uint8_t> out(str.begin(), str.end());
    serialized_ = out;
    return out;
}

void QuantumStateVector::clear_cache() {
    coherence_ = std::nullopt;
    cache_.clear();
    serialized_ = std::nullopt;
}

bool QuantumStateVector::operator==(const QuantumStateVector& other) const {
    return coordinates_ == other.coordinates_;
}

std::size_t QuantumStateVector::hash() const {
    std::hash<std::string> hasher;
    return hasher(std::string(reinterpret_cast<const char*>(serialize().data()), serialize().size()));
}

const std::vector<QuantumStateVector::Complex>& QuantumStateVector::coordinates() const {
    return coordinates_;
}

} // namespace hydra::qzkp
