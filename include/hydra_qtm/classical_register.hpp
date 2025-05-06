#pragma once
#include <vector>

namespace hydra::qtm {

class ClassicalRegister {
public:
    explicit ClassicalRegister(size_t num_bits);

    void set_bit(size_t index, bool value);
    bool get_bit(size_t index) const;
    std::vector<bool> dump() const;

private:
    std::vector<bool> bits_;
};

} // namespace hydra::qtm
