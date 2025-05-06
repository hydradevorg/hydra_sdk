cmake_minimum_required(VERSION 3.16)
project(hydra_kernel VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)

add_library(hydra_qtm SHARED
    src/hydra_qtm/quantum_state.cpp
    src/hydra_qtm/quantum_gate.cpp
    src/hydra_qtm/quantum_circuit.cpp
    src/hydra_qtm/simulator.cpp
    # optional components if split
)

target_include_directories(hydra_qtm PUBLIC include)
target_link_libraries(hydra_qtm PRIVATE Eigen3::Eigen)

enable_testing()

add_executable(hydra_qtm_tests
    tests/simulator_test.cpp
)

target_link_libraries(hydra_qtm_tests
    PRIVATE hydra_qtm
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hydra_qtm_tests)
