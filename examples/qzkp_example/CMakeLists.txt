cmake_minimum_required(VERSION 3.10)
project(qzkp_example VERSION 0.1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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
add_executable(qzkp_example ../qzkp_example.cpp)
target_link_libraries(qzkp_example PRIVATE hydra_qzkp)

# Installation
install(TARGETS qzkp_example
        RUNTIME DESTINATION bin)
