cmake_minimum_required(VERSION 3.16)
project(hydra_common VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create an interface library (header-only)
add_library(hydra_common INTERFACE)

# Include directories
target_include_directories(hydra_common INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

# Compile features
target_compile_features(hydra_common INTERFACE cxx_std_20)
