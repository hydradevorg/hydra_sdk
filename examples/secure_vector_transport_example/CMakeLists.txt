cmake_minimum_required(VERSION 3.10)
project(secure_vector_transport_example VERSION 0.1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(GMP REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Example executable
add_executable(secure_vector_transport_example ../secure_vector_transport_example.cpp)
target_link_libraries(secure_vector_transport_example
    lmvs
    hydra_crypto
    hydra_vfs
    hydra_compression_tvc
    hydra_compression_ost
    ${OPENSSL_LIBRARIES}
    GMP::GMP
)

# Installation
install(TARGETS secure_vector_transport_example
        RUNTIME DESTINATION bin)
