cmake_minimum_required(VERSION 3.16)
project(hydra_crypto VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Botan REQUIRED)
find_package(GMP REQUIRED)

# External dependencies for API server
include(FetchContent)

# cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.1
)
FetchContent_MakeAvailable(httplib)

# nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Library sources
set(HYDRA_CRYPTO_HEADERS
    ../../include/hydra_crypto/kyber_kem.hpp
    ../../include/hydra_crypto/dilithium_signature.hpp
    ../../include/hydra_crypto/kyber_aes.hpp
    ../../include/hydra_crypto/falcon_signature.hpp
    ../../include/hydra_crypto/root_key_manager.hpp
    ../../include/hydra_crypto/blake3_hash.hpp
    ../../include/hydra_crypto/hashing.hpp
)
set(HYDRA_CRYPTO_SOURCES
    kyber_kem.cpp
    dilithium_signature.cpp
    kyber_aes.cpp
    falcon_signature.cpp
    root_key_manager.cpp
    blake3_hash.cpp
    blake3_provider.cpp
    hashing.cpp
)

# Build both static and shared libraries
add_library(hydra_crypto STATIC ${HYDRA_CRYPTO_SOURCES} ${HYDRA_CRYPTO_HEADERS})
add_library(hydra_crypto_shared SHARED ${HYDRA_CRYPTO_SOURCES} ${HYDRA_CRYPTO_HEADERS})

# Set the output name for the shared library based on platform
if(WIN32)
    set_target_properties(hydra_crypto_shared PROPERTIES OUTPUT_NAME "hydra_crypto")
else()
    set_target_properties(hydra_crypto_shared PROPERTIES OUTPUT_NAME "hydra_crypto")
endif()

# Set version properties
set_target_properties(hydra_crypto_shared PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories for both libraries
target_include_directories(hydra_crypto
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_crypto
)
target_include_directories(hydra_crypto_shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_crypto
)

# Link libraries for both static and shared
target_link_libraries(hydra_crypto
    PUBLIC
        Botan::Botan
        OpenSSL::SSL
        OpenSSL::Crypto
        GMP::GMP
        blake3
)
target_link_libraries(hydra_crypto_shared
    PUBLIC
        Botan::Botan
        OpenSSL::SSL
        OpenSSL::Crypto
        GMP::GMP
        blake3
)

# Compile features for both libraries
target_compile_features(hydra_crypto PUBLIC cxx_std_20)
target_compile_features(hydra_crypto_shared PUBLIC cxx_std_20)

# Optionally install the libraries and headers
install(TARGETS hydra_crypto hydra_crypto_shared DESTINATION lib)
install(FILES ${HYDRA_CRYPTO_HEADERS} DESTINATION include/hydra_crypto)

# Main crypto demo (if needed)
add_executable(hydra_crypto_demo ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(hydra_crypto_demo PRIVATE hydra_crypto)

# GoogleTest
enable_testing()
find_package(GTest REQUIRED)

add_executable(test_falcon_signature ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_falcon_signature.cpp)
target_link_libraries(test_falcon_signature PRIVATE hydra_crypto GTest::GTest GTest::Main)
add_test(NAME FalconSignatureTest COMMAND test_falcon_signature)

add_executable(test_kyber_kem ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_kyber_kem.cpp)
target_link_libraries(test_kyber_kem PRIVATE hydra_crypto GTest::gtest_main)
add_test(NAME KyberKEMTest COMMAND test_kyber_kem)

add_executable(test_dilithium_signature ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_dilithium_signature.cpp)
target_link_libraries(test_dilithium_signature PRIVATE hydra_crypto GTest::gtest_main)
add_test(NAME DilithiumSignatureTest COMMAND test_dilithium_signature)
