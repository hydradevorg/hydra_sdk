cmake_minimum_required(VERSION 3.16)
project(hydra_address_examples VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the address generation example
add_executable(address_generation_example
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../examples/address_generation_example.cpp
)

# Include directories
target_include_directories(address_generation_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(address_generation_example
    PRIVATE
        hydra_address
        hydra_crypto
        hydra_math
        hydra_qzkp
        hydra_lmvs
)

# Install targets
install(TARGETS address_generation_example
    RUNTIME DESTINATION bin/examples
)
