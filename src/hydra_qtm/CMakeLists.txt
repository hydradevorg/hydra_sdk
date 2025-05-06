cmake_minimum_required(VERSION 3.16)
project(hydra_qtm VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)

add_library(hydra_qtm SHARED
    src/quantum_state.cpp
    src/quantum_gate.cpp
    src/quantum_circuit.cpp
    src/simulator.cpp
    src/qasm_export.cpp
    src/classical_register.cpp
    src/noise_model.cpp
    # optional components if split
)

target_include_directories(hydra_qtm
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(hydra_qtm PRIVATE Eigen3::Eigen)

enable_testing()

add_executable(hydra_qtm_tests
    tests/simulator_test.cpp
)

target_include_directories(hydra_qtm_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(hydra_qtm_tests
    PRIVATE hydra_qtm
    GTest::gtest_main
    Eigen3::Eigen
)

include(GoogleTest)
gtest_discover_tests(hydra_qtm_tests)
