#pragma once
#include <string>

namespace hydra::qtm {

enum class BackendType {
    DENSE,
    SPARSE,
    GPU,
    CUSTOM
};

struct BackendConfig {
    BackendType type = BackendType::DENSE;
    size_t seed = 42;
    size_t shots = 1024;
    bool debug = false;
};

} // namespace hydra::qtm
