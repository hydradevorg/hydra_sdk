cmake_minimum_required(VERSION 3.16)
project(hydra_qzkp VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(GMP REQUIRED)

# Find or fetch nlohmann_json
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(json)
endif()

# Library headers
set(HYDRA_QZKP_HEADERS
    ../../include/hydra_qzkp/qzkp.hpp
    ../../include/hydra_qzkp/quantum_state_vector.hpp
    ../../include/hydra_qzkp/result_cache.hpp
)

# Library sources
set(HYDRA_QZKP_SOURCES
    src/qzkp.cpp
    src/quantum_state_vector.cpp
    src/result_cache.cpp
)

# Build static library
add_library(hydra_qzkp STATIC ${HYDRA_QZKP_SOURCES})

# Include directories
target_include_directories(hydra_qzkp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
target_link_libraries(hydra_qzkp
    PUBLIC
        hydra_crypto
        hydra_math
        OpenSSL::SSL
        OpenSSL::Crypto
        GMP::GMP
        nlohmann_json::nlohmann_json
)

# Compile features
target_compile_features(hydra_qzkp PUBLIC cxx_std_17)

# Optionally install the library and headers
install(TARGETS hydra_qzkp DESTINATION lib)
install(FILES ${HYDRA_QZKP_HEADERS} DESTINATION include/hydra_qzkp)

# Enable testing
enable_testing()
find_package(GTest QUIET)

if(GTest_FOUND)
    # Add test executable
    add_executable(test_qzkp tests/test_qzkp.cpp)
    target_link_libraries(test_qzkp PRIVATE hydra_qzkp GTest::gtest_main)
    add_test(NAME QZKPTest COMMAND test_qzkp)
else()
    message(STATUS "GTest not found, skipping tests for hydra_qzkp")
endif()
