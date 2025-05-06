#include "hydra_qtm/classical_register.hpp"
#include <stdexcept>

namespace hydra::qtm {

ClassicalRegister::ClassicalRegister(size_t num_bits)
    : bits_(num_bits, false) {}

void ClassicalRegister::set_bit(size_t index, bool value) {
    if (index >= bits_.size())
        throw std::out_of_range("ClassicalRegister: index out of range");
    bits_[index] = value;
}

bool ClassicalRegister::get_bit(size_t index) const {
    if (index >= bits_.size())
        throw std::out_of_range("ClassicalRegister: index out of range");
    return bits_[index];
}

std::vector<bool> ClassicalRegister::dump() const {
    return bits_;
}

} // namespace hydra::qtm
