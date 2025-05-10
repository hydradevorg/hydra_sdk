cmake_minimum_required(VERSION 3.16)
project(hydra_address VERSION 1.0.0 LANGUAGES CXX)

# Find required packages
if(NOT HYDRA_WASM_BUILD)
  find_package(OpenSSL REQUIRED)
endif()

# Library headers
set(HYDRA_ADDRESS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/address_generator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/geohash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/layered_matrix.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/vector_compression.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/thread_safe_address_generator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/hydra_address/thread_safe_vector_compression.hpp
)

# Library sources
set(HYDRA_ADDRESS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/address_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geohash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layered_matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vector_compression.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lmvs_address_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_safe_address_generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_safe_vector_compression.cpp
)

# Build static library
add_library(hydra_address STATIC ${HYDRA_ADDRESS_SOURCES})

# Include directories
target_include_directories(hydra_address
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link libraries
if(HYDRA_WASM_BUILD)
  # WebAssembly build with minimal dependencies
  target_link_libraries(hydra_address
      PUBLIC
          hydra_crypto
  )

  # Add WebAssembly bindings if requested
  if(HYDRA_WASM_BINDINGS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wasm_build.cpp")
      # Use the simplified WebAssembly-specific implementation
      add_executable(hydra_address_wasm ${CMAKE_CURRENT_SOURCE_DIR}/wasm_build.cpp)
      set_target_properties(hydra_address_wasm PROPERTIES OUTPUT_NAME "hydra_address")

      # Set Emscripten-specific flags
      if(CMAKE_CXX_COMPILER MATCHES "em\\+\\+$")
        set_target_properties(hydra_address_wasm PROPERTIES
          LINK_FLAGS "-s WASM=1 -s EXPORT_ES6=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME=HydraModule"
        )
      endif()
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/wasm_bindings.cpp")
      # Fall back to the regular bindings
      add_executable(hydra_address_wasm ${CMAKE_CURRENT_SOURCE_DIR}/wasm_bindings.cpp)
      target_link_libraries(hydra_address_wasm PRIVATE hydra_address)
      set_target_properties(hydra_address_wasm PROPERTIES OUTPUT_NAME "hydra_address")

      # Set Emscripten-specific flags
      if(CMAKE_CXX_COMPILER MATCHES "em\\+\\+$")
        set_target_properties(hydra_address_wasm PROPERTIES
          LINK_FLAGS "-s WASM=1 -s EXPORT_ES6=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME=HydraModule"
        )
      endif()
    endif()
  endif()
else()
  # Native build with all dependencies
  target_link_libraries(hydra_address
      PUBLIC
          hydra_crypto
          hydra_math
          hydra_qzkp
          hydra_compression_tvc
          hydra_compression_ost
          hydra_lmvs
          ${OPENSSL_LIBRARIES}
          GMP::GMP
  )
endif()

# Install targets
install(TARGETS hydra_address
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HYDRA_ADDRESS_HEADERS}
    DESTINATION include/hydra_address
)

# Tests (skip for WebAssembly builds)
if(NOT HYDRA_WASM_BUILD)
  enable_testing()
  find_package(GTest REQUIRED)

  add_executable(test_thread_safety ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_thread_safety.cpp)
  target_link_libraries(test_thread_safety
      PRIVATE
          hydra_address
          hydra_crypto
          GTest::GTest
          GTest::Main
  )
  add_test(NAME ThreadSafetyTest COMMAND test_thread_safety)
endif()
