cmake_minimum_required(VERSION 3.16)
project(address_generation_example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the address generation example
add_executable(address_gen_example
    ${CMAKE_CURRENT_SOURCE_DIR}/../address_generation_example.cpp
)

# Include directories
target_include_directories(address_gen_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(address_gen_example
    PRIVATE
        hydra_address
        hydra_crypto
        hydra_math
        hydra_qzkp
        hydra_lmvs
)

# Set the output directory
set_target_properties(address_gen_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
