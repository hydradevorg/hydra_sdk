cmake_minimum_required(VERSION 3.10)
project(lmvs_bigint_example VERSION 0.1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(GMP REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Example executable
add_executable(lmvs_bigint_example ../lmvs_bigint_example.cpp)
target_link_libraries(lmvs_bigint_example
    lmvs
    GMP::GMP
    hydra_compression_tvc
    hydra_compression_ost
)

# Installation
install(TARGETS lmvs_bigint_example
        RUNTIME DESTINATION bin)
