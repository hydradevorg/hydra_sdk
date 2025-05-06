cmake_minimum_required(VERSION 3.10)
project(p2p_secure_vfs_example VERSION 0.1.0 LANGUAGES CXX C)

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

# Find additional required packages
find_package(OpenSSL REQUIRED)

# Example executable
add_executable(p2p_secure_vfs_example ../p2p_secure_vfs_example.cpp)
target_link_libraries(p2p_secure_vfs_example
    lmvs
    hydra_crypto
    hydra_vfs
    hydra_compression_tvc
    hydra_compression_ost
    ${OPENSSL_LIBRARIES}
    GMP::GMP
)

# Installation
install(TARGETS p2p_secure_vfs_example
        RUNTIME DESTINATION bin)
