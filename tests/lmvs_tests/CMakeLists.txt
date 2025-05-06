cmake_minimum_required(VERSION 3.10)
project(lmvs_tests VERSION 0.1.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Test executable
add_executable(lmvs_tests ../lmvs_tests.cpp)
target_link_libraries(lmvs_tests lmvs)

# Add test
add_test(NAME lmvs_tests COMMAND lmvs_tests)
