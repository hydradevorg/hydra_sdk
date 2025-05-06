cmake_minimum_required(VERSION 3.16)
project(hydra_address_tests VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTest
find_package(GTest REQUIRED)

# Add the address generator test
add_executable(test_address_generator
    test_address_generator.cpp
)

# Include directories
target_include_directories(test_address_generator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${GTEST_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(test_address_generator
    PRIVATE
        hydra_address
        hydra_crypto
        hydra_math
        hydra_qzkp
        hydra_lmvs
        ${GTEST_LIBRARIES}
        pthread
)

# Add the test
add_test(NAME test_address_generator COMMAND test_address_generator)

# Install targets
install(TARGETS test_address_generator
    RUNTIME DESTINATION bin/tests
)
