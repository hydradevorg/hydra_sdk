cmake_minimum_required(VERSION 3.16)
project(hydra_crypto VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find required packages
if(NOT HYDRA_WASM_BUILD)
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
else()
  # For WebAssembly builds, we use a simplified setup
  include(FetchContent)

  # nlohmann/json (needed for WebAssembly bindings)
  FetchContent_Declare(
      json
      GIT_REPOSITORY https://github.com/nlohmann/json.git
      GIT_TAG v3.11.3
  )
  FetchContent_MakeAvailable(json)
endif()

# Library sources
set(HYDRA_CRYPTO_HEADERS
    ../../include/hydra_crypto/kyber_kem.hpp
    ../../include/hydra_crypto/dilithium_signature.hpp
    ../../include/hydra_crypto/kyber_aes.hpp
    ../../include/hydra_crypto/falcon_signature.hpp
    ../../include/hydra_crypto/root_key_manager.hpp
    ../../include/hydra_crypto/blake3_hash.hpp
    ../../include/hydra_crypto/thread_safe_blake3_hash.hpp
    ../../include/hydra_crypto/hashing.hpp
)
set(HYDRA_CRYPTO_SOURCES
    kyber_kem.cpp
    dilithium_signature.cpp
    kyber_aes.cpp
    falcon_signature.cpp
    root_key_manager.cpp
    blake3_hash.cpp
    thread_safe_blake3_hash.cpp
    blake3_provider.cpp
    hashing.cpp
)

# Build libraries
if(HYDRA_WASM_BUILD)
    # For WebAssembly, only build the static library
    add_library(hydra_crypto STATIC ${HYDRA_CRYPTO_SOURCES})

    # Add WebAssembly bindings if requested
    if(HYDRA_WASM_BINDINGS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wasm_bindings.cpp")
        add_executable(hydra_crypto_wasm ${CMAKE_CURRENT_SOURCE_DIR}/wasm_bindings.cpp)
        target_link_libraries(hydra_crypto_wasm PRIVATE hydra_crypto)
        set_target_properties(hydra_crypto_wasm PROPERTIES OUTPUT_NAME "hydra_crypto")
    endif()
else()
    # For native builds, build both static and shared libraries
    add_library(hydra_crypto STATIC ${HYDRA_CRYPTO_SOURCES})
    add_library(hydra_crypto_shared SHARED ${HYDRA_CRYPTO_SOURCES})

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
endif()

# Include directories
target_include_directories(hydra_crypto
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_crypto
)

# Link libraries based on build type
if(HYDRA_WASM_BUILD)
    # Minimal dependencies for WebAssembly
    target_link_libraries(hydra_crypto
        PUBLIC
            nlohmann_json::nlohmann_json
    )

    # Add include directories for WebAssembly bindings if needed
    if(TARGET hydra_crypto_wasm)
        target_include_directories(hydra_crypto_wasm
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/../../include
                ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_crypto
        )
    endif()
else()
    # Full dependencies for native build
    target_link_libraries(hydra_crypto
        PUBLIC
            Botan::Botan
            OpenSSL::SSL
            OpenSSL::Crypto
            GMP::GMP
            blake3
    )

    # Configure shared library
    target_include_directories(hydra_crypto_shared
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/../../include
            ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_crypto
    )

    target_link_libraries(hydra_crypto_shared
        PUBLIC
            Botan::Botan
            OpenSSL::SSL
            OpenSSL::Crypto
            GMP::GMP
            blake3
    )

    # Compile features for shared library
    target_compile_features(hydra_crypto_shared PUBLIC cxx_std_20)
endif()

# Compile features for static library
target_compile_features(hydra_crypto PUBLIC cxx_std_20)

# Optionally install the libraries and headers
if(HYDRA_WASM_BUILD)
    if(TARGET hydra_crypto_wasm)
        install(TARGETS hydra_crypto_wasm DESTINATION wasm)
    endif()
    install(TARGETS hydra_crypto DESTINATION lib)
else()
    install(TARGETS hydra_crypto hydra_crypto_shared DESTINATION lib)

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
endif()

# Install headers in all cases
install(FILES ${HYDRA_CRYPTO_HEADERS} DESTINATION include/hydra_crypto)
