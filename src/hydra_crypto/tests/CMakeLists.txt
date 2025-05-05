# Test suite for hydra_crypto

# Use Catch2 for testing
find_package(Catch2 REQUIRED)

# Main test executable
set(TEST_SOURCES
    test_main.cpp
    test_dilithium_signature.cpp
    unit/test_parameter_validation.cpp
    integration/test_hydra_sdk_integration.cpp
)

add_executable(hydra_crypto_tests ${TEST_SOURCES})
target_link_libraries(hydra_crypto_tests PRIVATE 
    hydra_crypto
    Catch2::Catch2
)

# Create test directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(hydra_crypto_tests
    REPORTER XML
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/output
    OUTPUT_PREFIX "test_"
    OUTPUT_SUFFIX .xml
)

# Add simple test target that runs all tests
add_custom_target(run_tests
    COMMAND ${CMAKE_COMMAND} -E echo "Running all tests..."
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/hydra_crypto_tests
    DEPENDS hydra_crypto_tests
)

# Add test with code coverage (if supported)
option(WITH_COVERAGE "Enable coverage reporting" OFF)
if(WITH_COVERAGE)
    target_compile_options(hydra_crypto_tests PRIVATE --coverage)
    target_link_libraries(hydra_crypto_tests PRIVATE --coverage)
    
    # Add a target to generate coverage report
    find_program(GCOVR_PATH gcovr)
    if(GCOVR_PATH)
        add_custom_target(coverage
            COMMAND ${GCOVR_PATH} --xml-pretty --exclude-unreachable-branches --print-summary -r ${CMAKE_SOURCE_DIR} --object-directory=${CMAKE_BINARY_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/coverage.xml
            COMMAND ${GCOVR_PATH} --html-details --exclude-unreachable-branches -r ${CMAKE_SOURCE_DIR} --object-directory=${CMAKE_BINARY_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS run_tests
            COMMENT "Generating coverage report"
        )
    endif()
endif()
